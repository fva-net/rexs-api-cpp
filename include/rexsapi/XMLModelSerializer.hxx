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

#ifndef REXSAPI_XML_MODEL_SERIALIZER_HXX
#define REXSAPI_XML_MODEL_SERIALIZER_HXX

#include <rexsapi/CodedValue.hxx>
#include <rexsapi/Model.hxx>
#include <rexsapi/Xml.hxx>

namespace rexsapi
{
  /**
   * @brief Used to serialize a TModel in REXS xml format.
   *
   */
  class XMLModelSerializer
  {
  public:
    /**
     * @brief Serializes a TModel in REXS xml format.
     *
     * Uses the serializer to output the created xml object.
     *
     * @tparam TSerializer Serializer class for outputting an xml object. The TXMLStringSerializer and
     * TXMLFileSerializer classes are provided as default implementations. The serializer class has to define the
     * following method <br> ```void serialize(const pugi::xml_document& doc)```
     * @param model The model to serialize
     * @param serializer The serializer to output the serialized model with
     */
    template<typename TSerializer>
    void serialize(const TModel& model, TSerializer& serializer);

  private:
    void createDocument();

    pugi::xml_node serialize(const TModelInfo& info);

    void serialize(pugi::xml_node& relationsNode, const TRelations& relations);

    void serialize(pugi::xml_node& modelNode, const TComponents& components);

    pugi::xml_node serialize(pugi::xml_node& componentsNode, const TComponent& component);

    void serialize(pugi::xml_node& compNode, const TAttributes& attributes);

    void serialize(pugi::xml_node& attNode, const TAttribute& attribute);

    void serialize(pugi::xml_node& loadSpectrumNode, const TLoadSpectrum& loadSpectrum);

    std::string getNextComponentId() noexcept
    {
      return std::to_string(++m_ComponentId);
    }

    std::string getNextRelationId() noexcept
    {
      return std::to_string(++m_RelationId);
    }

    std::string getComponentId(uint64_t internalId) const;

    pugi::xml_document m_Doc;
    uint64_t m_ComponentId{0};
    uint64_t m_RelationId{0};
    std::unordered_map<uint64_t, std::string> m_ComponentMapping;
  };


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  template<typename TSerializer>
  inline void XMLModelSerializer::serialize(const TModel& model, TSerializer& serializer)
  {
    createDocument();
    auto models = serialize(model.getInfo());
    pugi::xml_node relationsNode = models.append_child("relations");
    serialize(models, model.getComponents());
    serialize(relationsNode, model.getRelations());

    if (model.getLoadSpectrum().hasLoadCases()) {
      pugi::xml_node loadSpectrum = models.append_child("load_spectrum");
      serialize(loadSpectrum, model.getLoadSpectrum());
    }

    serializer.serialize(m_Doc);
  }

  inline void XMLModelSerializer::createDocument()
  {
    m_Doc.reset();
    auto decl = m_Doc.append_child(pugi::node_declaration);
    decl.append_attribute("version") = "1.0";
    decl.append_attribute("encoding") = "UTF-8";
    decl.append_attribute("standalone") = "no";
  }

  inline pugi::xml_node XMLModelSerializer::serialize(const TModelInfo& info)
  {
    pugi::xml_node modelNode = m_Doc.append_child("model");
    modelNode.append_attribute("applicationId").set_value(info.getApplicationId().c_str());
    modelNode.append_attribute("applicationVersion").set_value(info.getApplicationVersion().c_str());
    modelNode.append_attribute("date").set_value(info.getDate().c_str());
    modelNode.append_attribute("version").set_value(info.getVersion().asString().c_str());
    if (info.getApplicationLanguage().has_value()) {
      modelNode.append_attribute("applicationLanguage").set_value(info.getApplicationLanguage()->c_str());
    }
    return modelNode;
  }

  inline void XMLModelSerializer::serialize(pugi::xml_node& relationsNode, const TRelations& relations)
  {
    for (const auto& relation : relations) {
      pugi::xml_node relNode = relationsNode.append_child("relation");
      relNode.append_attribute("id").set_value(getNextRelationId().c_str());
      relNode.append_attribute("type").set_value(toRelationTypeString(relation.getType()).c_str());
      if (relation.getOrder().has_value()) {
        relNode.append_attribute("order").set_value(relation.getOrder().value());
      }
      for (const auto& reference : relation.getReferences()) {
        pugi::xml_node refNode = relNode.append_child("ref");
        if (!reference.getHint().empty()) {
          refNode.append_attribute("hint").set_value(reference.getHint().c_str());
        }
        auto id = getComponentId(reference.getComponent().getInternalId());
        refNode.append_attribute("id").set_value(id.c_str());
        refNode.append_attribute("role").set_value(toRelationRoleString(reference.getRole()).c_str());
      }
    }
  }

  inline void XMLModelSerializer::serialize(pugi::xml_node& modelNode, const TComponents& components)
  {
    pugi::xml_node componentsNode = modelNode.append_child("components");
    for (const auto& component : components) {
      auto id = getNextComponentId();
      m_ComponentMapping.emplace(component.getInternalId(), id);
    }
    for (const auto& component : components) {
      auto compNode = serialize(componentsNode, component);
      serialize(compNode, component.getAttributes());
    }
  }

  inline pugi::xml_node XMLModelSerializer::serialize(pugi::xml_node& componentsNode, const TComponent& component)
  {
    pugi::xml_node compNode = componentsNode.append_child("component");
    compNode.append_attribute("id").set_value(getComponentId(component.getInternalId()).c_str());
    compNode.append_attribute("name").set_value(component.getName().c_str());
    compNode.append_attribute("type").set_value(component.getType().c_str());
    return compNode;
  }

  inline void XMLModelSerializer::serialize(pugi::xml_node& compNode, const TAttributes& attributes)
  {
    for (const auto& attribute : attributes) {
      pugi::xml_node attNode = compNode.append_child("attribute");
      attNode.append_attribute("id").set_value(attribute.getAttributeId().c_str());
      attNode.append_attribute("unit").set_value(attribute.getUnit().getName().c_str());
      serialize(attNode, attribute);
    }
  }

  template<typename T>
  inline void xmlEncodeCodedArray(pugi::xml_node& attNode, const TValue& value, const std::vector<T>& array,
                                  std::function<std::string(typename std::vector<T>::value_type)>&& formatter)
  {
    auto arrayNode = attNode.append_child("array");

    if (value.coded() != TCodeType::None) {
      const auto [val, code] = detail::encodeArray(array, value.coded());
      arrayNode.append_attribute("code").set_value(detail::toCodedValueString(code).c_str());
      arrayNode.append_child(pugi::node_pcdata).set_value(val.c_str());
    } else {
      for (const T& element : array) {
        auto child = arrayNode.append_child("c");
        child.append_child(pugi::node_pcdata).set_value(formatter(element).c_str());
      }
    }
  }

  template<typename T>
  inline void xmlEncodeCodedMatrix(pugi::xml_node& attNode, const TValue& value, const TMatrix<T>& matrix,
                                   std::function<std::string(typename TMatrix<T>::value_type)>&& formatter)
  {
    auto matrixNode = attNode.append_child("matrix");

    if (value.coded() != TCodeType::None) {
      const auto [val, code] = detail::encodeMatrix(matrix, value.coded());
      matrixNode.append_attribute("code").set_value(detail::toCodedValueString(code).c_str());
      matrixNode.append_attribute("rows").set_value(matrix.m_Values.size());
      matrixNode.append_attribute("columns").set_value(matrix.m_Values.size());
      matrixNode.append_child(pugi::node_pcdata).set_value(val.c_str());
    } else {
      for (const auto& row : matrix.m_Values) {
        auto rowNode = matrixNode.append_child("r");
        for (const auto& column : row) {
          auto child = rowNode.append_child("c");
          child.append_child(pugi::node_pcdata).set_value(formatter(column).c_str());
        }
      }
    }
  }

  inline void XMLModelSerializer::serialize(pugi::xml_node& attNode, const TAttribute& attribute)
  {
    if (attribute.getValue().isEmpty()) {
      return;
    }

    rexsapi::dispatch<void>(
      attribute.getValueType(), attribute.getValue(),
      {[&attNode](rexsapi::TFloatTag, const auto& d) -> void {
         attNode.append_child(pugi::node_pcdata).set_value(format(d).c_str());
       },
       [&attNode](rexsapi::TBoolTag, const auto& b) -> void {
         attNode.append_child(pugi::node_pcdata).set_value(fmt::format("{}", b).c_str());
       },
       [&attNode](rexsapi::TIntTag, const auto& i) -> void {
         attNode.append_child(pugi::node_pcdata).set_value(fmt::format("{}", i).c_str());
       },
       [&attNode](rexsapi::TEnumTag, const auto& s) -> void {
         attNode.append_child(pugi::node_pcdata).set_value(s.c_str());
       },
       [&attNode](rexsapi::TStringTag, const auto& s) -> void {
         attNode.append_child(pugi::node_pcdata).set_value(s.c_str());
       },
       [&attNode](rexsapi::TFileReferenceTag, const auto& s) -> void {
         attNode.append_child(pugi::node_pcdata).set_value(s.c_str());
       },
       [&attNode](rexsapi::TDatetimeTag, const auto& d) -> void {
         attNode.append_child(pugi::node_pcdata).set_value(d.asUTCString().data());
       },
       [&attNode, &attribute](rexsapi::TFloatArrayTag, const auto& a) -> void {
         xmlEncodeCodedArray(attNode, attribute.getValue(), a, [](double element) {
           return format(element);
         });
       },
       [&attNode](rexsapi::TBoolArrayTag, const auto& a) -> void {
         auto arrayNode = attNode.append_child("array");
         for (const auto& element : a) {
           auto child = arrayNode.append_child("c");
           child.append_child(pugi::node_pcdata).set_value(fmt::format("{}", element.m_Value).c_str());
         }
       },
       [&attNode, &attribute](rexsapi::TIntArrayTag, const auto& a) -> void {
         xmlEncodeCodedArray(attNode, attribute.getValue(), a, [](auto element) {
           return fmt::format("{}", element);
         });
       },
       [&attNode](rexsapi::TEnumArrayTag, const auto& a) -> void {
         auto arrayNode = attNode.append_child("array");
         for (const auto& element : a) {
           auto child = arrayNode.append_child("c");
           child.append_child(pugi::node_pcdata).set_value(fmt::format("{}", element).c_str());
         }
       },
       [&attNode](rexsapi::TStringArrayTag, const auto& a) -> void {
         auto arrayNode = attNode.append_child("array");
         for (const auto& element : a) {
           auto child = arrayNode.append_child("c");
           child.append_child(pugi::node_pcdata).set_value(fmt::format("{}", element).c_str());
         }
       },
       [&attNode](rexsapi::TReferenceComponentTag, const auto& n) -> void {
         attNode.append_child(pugi::node_pcdata).set_value(fmt::format("{}", n).c_str());
       },
       [&attNode, &attribute](rexsapi::TFloatMatrixTag, const auto& m) -> void {
         xmlEncodeCodedMatrix(attNode, attribute.getValue(), m, [](auto element) {
           return format(element);
         });
       },
       [&attNode, &attribute](rexsapi::TIntMatrixTag, const auto& m) -> void {
         xmlEncodeCodedMatrix(attNode, attribute.getValue(), m, [](auto element) {
           return format(static_cast<double>(element));
         });
       },
       [&attNode](rexsapi::TBoolMatrixTag, const auto& m) -> void {
         auto matrixNode = attNode.append_child("matrix");
         for (const auto& row : m.m_Values) {
           auto rowNode = matrixNode.append_child("r");
           for (const auto& column : row) {
             auto child = rowNode.append_child("c");
             child.append_child(pugi::node_pcdata).set_value(fmt::format("{}", *column).c_str());
           }
         }
       },
       [&attNode](rexsapi::TStringMatrixTag, const auto& m) -> void {
         auto matrixNode = attNode.append_child("matrix");
         for (const auto& row : m.m_Values) {
           auto rowNode = matrixNode.append_child("r");
           for (const auto& column : row) {
             auto child = rowNode.append_child("c");
             child.append_child(pugi::node_pcdata).set_value(fmt::format("{}", column).c_str());
           }
         }
       },
       [&attNode](rexsapi::TArrayOfIntArraysTag, const auto& a) -> void {
         auto arraysNode = attNode.append_child("array_of_arrays");
         for (const auto& array : a) {
           auto aNode = arraysNode.append_child("array");
           for (const auto& c : array) {
             auto child = aNode.append_child("c");
             child.append_child(pugi::node_pcdata).set_value(fmt::format("{}", c).c_str());
           }
         }
       }});
  }

  inline void XMLModelSerializer::serialize(pugi::xml_node& loadSpectrumNode, const TLoadSpectrum& loadSpectrum)
  {
    loadSpectrumNode.append_attribute("id").set_value("1");

    uint64_t loadCaseId{0};
    for (const auto& loadCase : loadSpectrum.getLoadCases()) {
      pugi::xml_node loadCaseNode = loadSpectrumNode.append_child("load_case");
      loadCaseNode.append_attribute("id").set_value(std::to_string(++loadCaseId).c_str());
      for (const auto& loadComponent : loadCase.getLoadComponents()) {
        const auto& component = loadComponent.getComponent();
        pugi::xml_node compNode = loadCaseNode.append_child("component");
        auto id = getComponentId(component.getInternalId());
        compNode.append_attribute("id").set_value(id.c_str());
        if (!component.getName().empty()) {
          compNode.append_attribute("name").set_value(component.getName().c_str());
        }
        compNode.append_attribute("type").set_value(component.getType().c_str());
        serialize(compNode, loadComponent.getLoadAttributes());
      }
    }

    if (loadSpectrum.hasAccumulation()) {
      pugi::xml_node accumulationNode = loadSpectrumNode.append_child("accumulation");
      for (const auto& loadComponent : loadSpectrum.getAccumulation().getLoadComponents()) {
        const auto& component = loadComponent.getComponent();
        pugi::xml_node compNode = accumulationNode.append_child("component");
        auto id = getComponentId(component.getInternalId());
        compNode.append_attribute("id").set_value(id.c_str());
        if (!component.getName().empty()) {
          compNode.append_attribute("name").set_value(component.getName().c_str());
        }
        compNode.append_attribute("type").set_value(component.getType().c_str());
        serialize(compNode, loadComponent.getLoadAttributes());
      }
    }
  }

  inline std::string XMLModelSerializer::getComponentId(uint64_t internalId) const
  {
    auto it = m_ComponentMapping.find(internalId);
    if (it == m_ComponentMapping.end()) {
      throw TException{fmt::format("cannot find referenced component with id {}", internalId)};
    }
    return it->second;
  }
}

#endif
