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

#ifndef REXSAPI_XML_MODEL_LOADER_HXX
#define REXSAPI_XML_MODEL_LOADER_HXX

#include <rexsapi/ConversionHelper.hxx>
#include <rexsapi/ModelHelper.hxx>
#include <rexsapi/RelationTypeChecker.hxx>
#include <rexsapi/XMLValueDecoder.hxx>
#include <rexsapi/XSDSchemaValidator.hxx>
#include <rexsapi/XmlUtils.hxx>
#include <rexsapi/database/ModelRegistry.hxx>

#include <set>

namespace rexsapi
{
  /**
   * @brief Creates TModel instances from REXS xml buffer
   *
   * A xml model loader can create TModel instances from supplied buffer. The buffer has to contain a REXS model in
   * xml format that validates against the xsd schema. Additionally, a mode can be chosen in order to define how to
   * handle issues while creating the model. If the mode is TMode::RELAXED_MODE, most errors will be downgraded to
   * warnings.
   *
   */
  class TXMLModelLoader
  {
  public:
    /**
     * @brief Constructs a new TXMLModelLoader object
     *
     * @param mode Defines how to handle encountered issues while processing a model buffer
     * @param validator The xsd schema for validating the buffer
     */
    explicit TXMLModelLoader(TMode mode, const TXSDSchemaValidator& validator)
    : m_Mode{mode}
    , m_Validator{validator}
    , m_LoaderHelper{mode}
    {
    }

    /**
     * @brief Processes a buffer and creates a TModel instance upon success
     *
     * Will first validate the buffer against the xsd schema. Only valid buffer will be processed.
     *
     * @param result Describes the outcome of the operation. Will contain messages upon issues encountered.
     * @param registry Will load the REXS database version and language corresponding to the version information in the
     * buffer
     * @param buffer The actual REXS model in xml format
     * @return std::optional<TModel> Will contain a TModel instance if one could be created. Can be empty if critical
     * errors are encountered while processing the buffer. Buffer not validating against the schema are one source of
     * critical errors.
     */
    std::optional<TModel> load(TResult& result, const database::TModelRegistry& registry,
                               std::vector<uint8_t>& buffer) const;

  private:
    static bool checkDuplicate(const TAttributes& attributes, const TAttribute& attribute);

    TAttributes getAttributes(const std::string& context, TResult& result, uint64_t componentId,
                              const database::TComponent& componentType,
                              const pugi::xpath_node_set& attributeNodes) const;

    detail::TModeAdapter m_Mode;
    const TXSDSchemaValidator& m_Validator;
    detail::TModelHelper<detail::TXMLValueDecoder> m_LoaderHelper;
  };


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  inline std::optional<TModel> TXMLModelLoader::load(TResult& result, const database::TModelRegistry& registry,
                                                     std::vector<uint8_t>& buffer) const
  {
    const pugi::xml_document doc = detail::loadXMLDocument(result, buffer, m_Validator);
    if (!result) {
      return {};
    }

    const auto rexsModel = *doc.select_nodes("/model").begin();
    const auto language = detail::getStringAttribute(rexsModel, "applicationLanguage", "");
    const TModelInfo info{detail::getStringAttribute(rexsModel, "applicationId"),
                          detail::getStringAttribute(rexsModel, "applicationVersion"),
                          detail::getStringAttribute(rexsModel, "date"),
                          TRexsVersion{detail::getStringAttribute(rexsModel, "version")},
                          language.empty() ? std::optional<std::string>{} : language};

    const auto& dbModel =
      registry.getModel(info.getVersion(), language.empty() ? "en" : language, m_Mode.getMode() == TMode::STRICT_MODE);

    if (dbModel.getVersion() != info.getVersion()) {
      result.addError(TError{TErrorLevel::WARN, fmt::format("exact database model for version not available, using {}",
                                                            dbModel.getVersion().asString())});
    }

    detail::ComponentMapping componentsMapping;
    TComponents components;
    components.reserve(10);
    std::set<uint64_t> usedComponents;

    for (const auto& component : doc.select_nodes("/model/components/component")) {
      const auto componentId = convertToUint64(detail::getStringAttribute(component, "id"));
      std::string componentName = detail::getStringAttribute(component, "name", "");
      try {
        const auto& componentType = dbModel.findComponentById(detail::getStringAttribute(component, "type"));

        const auto attributeNodes =
          doc.select_nodes(fmt::format("/model/components/component[@id = '{}']/attribute", componentId).c_str());
        std::string context = componentName.empty() ? componentType.getName() : componentName;
        TAttributes attributes = getAttributes(context, result, componentId, componentType, attributeNodes);

        components.emplace_back(TComponent{componentId, componentsMapping.addComponent(componentId), componentType,
                                           componentName, std::move(attributes)});
      } catch (const std::exception& ex) {
        result.addError(
          TError{m_Mode.adapt(TErrorLevel::ERR), fmt::format("component id={}: {}", componentId, ex.what())});
      }
    }
    detail::ComponentPostProcessor postProcessor{result, m_Mode, components, componentsMapping};
    components = postProcessor.release();

    TRelations relations;
    for (const auto& relation : doc.select_nodes("/model/relations/relation")) {
      std::string relationId = detail::getStringAttribute(relation, "id");
      try {
        auto relationType = relationTypeFromString(detail::getStringAttribute(relation, "type"));
        std::optional<uint32_t> order;
        if (const auto orderAtt = relation.node().attribute("order"); !orderAtt.empty()) {
          order = orderAtt.as_uint();
          if (order.value() < 1) {
            result.addError(
              TError{m_Mode.adapt(TErrorLevel::ERR), fmt::format("relation id={} order is <1", relationId)});
          }
        }

        TRelationReferences references;
        for (const auto& reference :
             doc.select_nodes(fmt::format("/model/relations/relation[@id = '{}']/ref", relationId).c_str())) {
          std::string referenceId = detail::getStringAttribute(reference, "id");
          try {
            auto role = relationRoleFromString(detail::getStringAttribute(reference, "role"));
            std::string hint = detail::getStringAttribute(reference, "hint", "");

            const auto* component = componentsMapping.getComponent(convertToUint64(referenceId), components);
            if (component == nullptr) {
              result.addError(TError{
                m_Mode.adapt(TErrorLevel::ERR),
                fmt::format("relation id={} referenced component id={} does not exist", relationId, referenceId)});
              continue;
            }
            usedComponents.emplace(component->getInternalId());
            references.emplace_back(TRelationReference{role, hint, *component});
          } catch (const std::exception& ex) {
            result.addError(TError{m_Mode.adapt(TErrorLevel::ERR),
                                   fmt::format("cannot process relation reference id={}: {}", referenceId, ex.what())});
          }
        }

        relations.emplace_back(TRelation{relationType, order, std::move(references)});
      } catch (const std::exception& ex) {
        result.addError(TError{m_Mode.adapt(TErrorLevel::ERR),
                               fmt::format("cannot process relation id={}: {}", relationId, ex.what())});
      }
    }
    if (usedComponents.size() != components.size()) {
      result.addError(TError{TErrorLevel::WARN, fmt::format("{} components are not used in a relation",
                                                            components.size() - usedComponents.size())});
    }

    TLoadCases loadCases;
    {
      for (const auto& loadCase : doc.select_nodes("/model/load_spectrum/load_case")) {
        std::string loadCaseId = detail::getStringAttribute(loadCase, "id");
        TLoadComponents loadComponents;

        for (const auto& component : doc.select_nodes(
               fmt::format("/model/load_spectrum/load_case[@id = '{}']/component", loadCaseId).c_str())) {
          auto componentId = convertToUint64(detail::getStringAttribute(component, "id"));
          try {
            const auto* refComponent = componentsMapping.getComponent(componentId, components);
            if (refComponent == nullptr) {
              result.addError(
                TError{m_Mode.adapt(TErrorLevel::ERR),
                       fmt::format("load_case id={} component id={} does not exist", loadCaseId, componentId)});
              continue;
            }

            const auto attributeNodes =
              doc.select_nodes(fmt::format("/model/load_spectrum/load_case[@id = '{}']/component[@id = '{}']/attribute",
                                           loadCaseId, componentId)
                                 .c_str());
            const auto context = fmt::format("load_case id={}", loadCaseId);
            TAttributes attributes = getAttributes(context, result, componentId,
                                                   dbModel.findComponentById(refComponent->getType()), attributeNodes);
            loadComponents.emplace_back(TLoadComponent(*refComponent, std::move(attributes)));
          } catch (const std::exception& ex) {
            result.addError(TError{m_Mode.adapt(TErrorLevel::ERR), fmt::format("load_case id={} component id={}: {}",
                                                                               loadCaseId, componentId, ex.what())});
          }
        }
        loadCases.emplace_back(std::move(loadComponents));
      }
    }
    std::optional<TAccumulation> accumulation;
    {
      TLoadComponents loadComponents;
      for (const auto& component : doc.select_nodes("/model/load_spectrum/accumulation/component")) {
        auto componentId = convertToUint64(detail::getStringAttribute(component, "id"));
        try {
          const auto* refComponent = componentsMapping.getComponent(componentId, components);
          if (refComponent == nullptr) {
            result.addError(TError{m_Mode.adapt(TErrorLevel::ERR),
                                   fmt::format("accumulation component id={} does not exist", componentId)});
            continue;
          }

          const auto attributeNodes = doc.select_nodes(
            fmt::format("/model/load_spectrum/accumulation/component[@id = '{}']/attribute", componentId).c_str());
          TAttributes attributes = getAttributes("accumulation", result, componentId,
                                                 dbModel.findComponentById(refComponent->getType()), attributeNodes);
          loadComponents.emplace_back(TLoadComponent(*refComponent, std::move(attributes)));
        } catch (const std::exception& ex) {
          result.addError(TError{m_Mode.adapt(TErrorLevel::ERR),
                                 fmt::format("accumulation component id={}: {}", componentId, ex.what())});
        }
      }
      if (!loadComponents.empty()) {
        accumulation = TAccumulation{std::move(loadComponents)};
      }
    }

    TModel model{info, std::move(components), std::move(relations),
                 TLoadSpectrum{std::move(loadCases), std::move(accumulation)}};
    TRelationTypeChecker checker{m_Mode.getMode()};
    checker.check(result, model);
    return model;
  }

  inline bool TXMLModelLoader::checkDuplicate(const TAttributes& attributes, const TAttribute& attribute)
  {
    auto it = std::find_if(attributes.begin(), attributes.end(), [&attribute](const auto& att) {
      return attribute.getAttributeId() == att.getAttributeId();
    });
    return it != attributes.end();
  }

  inline TAttributes TXMLModelLoader::getAttributes(const std::string& context, TResult& result,
                                                    uint64_t componentId,
                                                    const database::TComponent& componentType,
                                                    const pugi::xpath_node_set& attributeNodes) const
  {
    TAttributes attributes;
    for (const auto& attribute : attributeNodes) {
      std::string id = detail::getStringAttribute(attribute, "id");
      auto unit = detail::getStringAttribute(attribute, "unit");

      bool isCustom = m_LoaderHelper.checkCustom(result, context, id, componentId, componentType);

      if (!isCustom) {
        const auto& att = componentType.findAttributeById(id);

        if (unit.empty()) {
          unit = att.getUnit().getName();
        } else {
          if (!att.getUnit().compare(unit)) {
            result.addError(
              TError{m_Mode.adapt(TErrorLevel::ERR),
                     fmt::format("{}: attribute id={} of component id={} does not specify the correct unit: '{}'",
                                 context, id, componentId, unit)});
          }
        }

        auto value = m_LoaderHelper.getValue(result, context, id, componentId, att, attribute.node());
        TAttribute newAttribute{att, std::move(value)};
        if (checkDuplicate(attributes, newAttribute)) {
          result.addError(TError{m_Mode.adapt(TErrorLevel::ERR),
                                 fmt::format("{}: duplicate attribute found for attribute id={} of component id={}",
                                             context, id, componentId)});
        }
        attributes.emplace_back(std::move(newAttribute));
      } else {
        auto [value, type] = m_LoaderHelper.getDecoder().decodeUnknown(attribute.node());
        attributes.emplace_back(TAttribute{id, TUnit{unit}, type, std::move(value)});
      }
    }

    return attributes;
  }
}

#endif
