/*
 * Copyright Schaeffler Technologies AG & Co. KG (info.de@schaeffler.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef REXSAPI_DATABASE_XML_MODEL_LOADER_HXX
#define REXSAPI_DATABASE_XML_MODEL_LOADER_HXX

#include <rexsapi/ConversionHelper.hxx>
#include <rexsapi/XSDSchemaValidator.hxx>
#include <rexsapi/XmlUtils.hxx>
#include <rexsapi/database/ComponentAttributeMapper.hxx>

#include <cstring>

/** @file */

namespace rexsapi::database
{
  /**
   * @brief Loads REXS database models in XML format
   *
   * Is used to populate the REXS database model registry.
   *
   * @tparam TResourceLoader Loader class for loading XML resources. The TFileResourceLoader class is
   * provided as default implementation. The loader class has to define the following method <br> ```TResult
   * load(const std::function<void(TResult&, std::vector<uint8_t>&)>& callback) const```
   * @tparam TSchemaLoader Loader class for loading XML schema resources. The TFileXsdSchemaLoader and
   * TBufferXsdSchemaLoader classes are provided as default implementations. The loader class has to define the
   * following method <br> ```pugi::xml_document load() const```
   */
  template<typename TResourceLoader, typename TSchemaLoader>
  class TXmlModelLoader
  {
  public:
    /**
     * @brief Constructs a new TXmlModelLoader object
     *
     * @param resourceLoader Resource loader instance to use for model loading
     * @param schemaLoader Resource loader instance to use for schema loading
     */
    explicit TXmlModelLoader(const TResourceLoader& resourceLoader, const TSchemaLoader& schemaLoader)
    : m_ResourceLoader{resourceLoader}
    , m_SchemaLoader{schemaLoader}
    {
    }

    /**
     * @brief Loads REXS database models
     *
     * Uses the resource loader to load XML database models into a buffer. Then uses the schema loader to load the REXS
     * database model XSD schema and checks all loaded model buffers against the schema. Finally, creates a model from
     * each buffer and hands it to the callback. Issues while loading and processing buffers will be added to the
     * returned result.
     * @param callback Will be called for each created model for further processing
     * @return TResult describing the outcome of the loading
     */
    TResult load(const std::function<void(TModel&&)>& callback) const;

  private:
    std::optional<TInterval> readInterval(const pugi::xpath_node& node) const;

    const TResourceLoader& m_ResourceLoader;
    const TSchemaLoader& m_SchemaLoader;
  };


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  template<typename TResourceLoader, typename TSchemaLoader>
  inline TResult
  TXmlModelLoader<TResourceLoader, TSchemaLoader>::load(const std::function<void(TModel&&)>& callback) const
  {
    return m_ResourceLoader.load([this, &callback](TResult& result, std::vector<uint8_t>& buffer) {
      pugi::xml_document doc = rexsapi::detail::loadXMLDocument(result, buffer, TXSDSchemaValidator{m_SchemaLoader});
      if (!result) {
        return;
      }

      const auto rexsModel = *doc.select_nodes("/rexsModel").begin();
      TModel model{TRexsVersion{rexsapi::detail::getStringAttribute(rexsModel, "version")},
                   rexsapi::detail::getStringAttribute(rexsModel, "language"),
                   rexsapi::detail::getStringAttribute(rexsModel, "date"),
                   statusFromString(rexsapi::detail::getStringAttribute(rexsModel, "status"))};

      for (const auto& node : doc.select_nodes("/rexsModel/units/unit")) {
        auto id = convertToUint64(rexsapi::detail::getStringAttribute(node, "id"));
        auto name = rexsapi::detail::getStringAttribute(node, "name");
        model.addUnit(TUnit{id, name});
      }

      for (const auto& node : doc.select_nodes("/rexsModel/valueTypes/valueType")) {
        auto id = convertToUint64(rexsapi::detail::getStringAttribute(node, "id"));
        auto name = rexsapi::detail::getStringAttribute(node, "name");
        model.addType(id, typeFromString(name));
      }

      for (const auto& node : doc.select_nodes("/rexsModel/attributes/attribute")) {
        auto attributeId = rexsapi::detail::getStringAttribute(node, "attributeId");
        auto name = rexsapi::detail::getStringAttribute(node, "name");
        auto valueType =
          model.findValueTypeById(convertToUint64(rexsapi::detail::getStringAttribute(node, "valueType")));
        auto unit = convertToUint64(rexsapi::detail::getStringAttribute(node, "unit"));
        std::string symbol = rexsapi::detail::getStringAttribute(node, "symbol", "");

        std::optional<TInterval> interval = readInterval(node);

        std::optional<TEnumValues> enumValues;
        if (const auto& enums = node.node().first_child();
            (valueType == TValueType::ENUM || valueType == TValueType::ENUM_ARRAY) && !enums.empty() &&
            std::strncmp(enums.name(), "enumValues", ::strlen("enumValues")) == 0) {
          std::vector<TEnumValue> values;
          for (const auto& value : enums.children()) {
            auto enumValue = rexsapi::detail::getStringAttribute(value, "value");
            auto enumName = rexsapi::detail::getStringAttribute(value, "name");
            values.emplace_back(TEnumValue{enumValue, enumName});
          }
          enumValues = TEnumValues{std::move(values)};
        }

        model.addAttribute(
          TAttribute{attributeId, name, valueType, model.findUnitById(unit), symbol, interval, enumValues});
      }

      std::vector<std::pair<std::string, std::string>> attributeMappings;
      for (const auto& node : doc.select_nodes("/rexsModel/componentAttributeMappings/componentAttributeMapping")) {
        auto componentId = rexsapi::detail::getStringAttribute(node, "componentId");
        auto attributeId = rexsapi::detail::getStringAttribute(node, "attributeId");
        attributeMappings.emplace_back(componentId, attributeId);
      }
      rexsapi::database::detail::TComponentAttributeMapper attributeMapper{model, std::move(attributeMappings)};

      for (const auto& node : doc.select_nodes("/rexsModel/components/component")) {
        auto id = rexsapi::detail::getStringAttribute(node, "componentId");
        auto name = rexsapi::detail::getStringAttribute(node, "name");
        auto attributes = attributeMapper.getAttributesForComponent(id);
        model.addComponent(TComponent{id, name, std::move(attributes)});
      }

      callback(std::move(model));
    });
  }

  template<typename TResourceLoader, typename TSchemaLoader>
  std::optional<TInterval>
  TXmlModelLoader<TResourceLoader, TSchemaLoader>::readInterval(const pugi::xpath_node& node) const
  {
    std::optional<TInterval> interval;

    TIntervalEndpoint min;
    TIntervalEndpoint max;

    if (auto att = node.node().attribute("rangeMin"); !att.empty()) {
      auto open = rexsapi::detail::getBoolAttribute(node, "rangeMinIntervalOpen", true);
      min = TIntervalEndpoint{convertToDouble(att.value()), open ? TIntervalType::OPEN : TIntervalType::CLOSED};
    }
    if (auto att = node.node().attribute("rangeMax"); !att.empty()) {
      auto open = rexsapi::detail::getBoolAttribute(node, "rangeMaxIntervalOpen", true);
      max = TIntervalEndpoint{convertToDouble(att.value()), open ? TIntervalType::OPEN : TIntervalType::CLOSED};
    }

    if (max.isSet() || min.isSet()) {
      interval = TInterval{min, max};
    }

    return interval;
  }
}

#endif
