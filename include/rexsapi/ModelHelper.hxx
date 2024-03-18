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

#ifndef REXSAPI_MODEL_HELPER_HXX
#define REXSAPI_MODEL_HELPER_HXX

#include <rexsapi/Mode.hxx>
#include <rexsapi/Model.hxx>
#include <rexsapi/ValidityChecker.hxx>
#include <rexsapi/database/Component.hxx>

#include <algorithm>
#include <unordered_map>

namespace rexsapi::detail
{
  template<typename ValueDecoderType>
  class TModelHelper
  {
  public:
    explicit TModelHelper(TMode mode)
    : m_Mode{mode}
    {
    }

    bool checkCustom(TResult& result, std::string_view context, const std::string& attributeId, uint64_t componentId,
                     const database::TComponent& componentType) const noexcept
    {
      bool isCustom{false};
      if (attributeId.substr(0, 7) == "custom_") {
        isCustom = true;
      } else {
        if (!componentType.hasAttribute(attributeId)) {
          isCustom = true;
          result.addError(TError{m_Mode.adapt(TErrorLevel::ERR),
                                 fmt::format("{}: attribute id={} is not part of component {} id={}", context,
                                             attributeId, componentType.getComponentId(), componentId)});
        }
      }
      return isCustom;
    }

    template<typename NodeType>
    TValue getValue(TResult& result, std::string_view context, std::string_view attributeId, uint64_t componentId,
                    const database::TAttribute& dbAttribute, const NodeType& attribute) const noexcept
    {
      auto [value, res] = m_Decoder.decode(dbAttribute.getValueType(), dbAttribute.getEnums(), attribute);
      auto decodedValue = checkResult(result, std::move(value), res, context, attributeId, componentId);
      if (!decodedValue.isEmpty() && !TValidityChecker::check(dbAttribute, decodedValue)) {
        result.addError(TError{m_Mode.adapt(TErrorLevel::ERR),
                               fmt::format("{}: value is out of range for attribute id={} of component id={}", context,
                                           attributeId, componentId)});
      }
      return decodedValue;
    }

    template<typename NodeType>
    TValue getValue(TResult& result, TValueType valueType, std::string_view context, std::string_view attributeId,
                    uint64_t componentId, const NodeType& attribute) const noexcept
    {
      auto [value, res] = m_Decoder.decode(valueType, {}, attribute);
      return checkResult(result, std::move(value), res, context, attributeId, componentId);
    }

    const ValueDecoderType& getDecoder() const noexcept
    {
      return m_Decoder;
    }

  private:
    TValue checkResult(TResult& result, TValue value, TDecoderResult res, std::string_view context,
                       std::string_view attributeId, uint64_t componentId) const noexcept
    {
      if (res == TDecoderResult::WRONG_TYPE) {
        result.addError(
          TError{m_Mode.adapt(TErrorLevel::ERR),
                 fmt::format("{}: value of attribute id={} of component id={} does not have the correct value type",
                             context, attributeId, componentId)});
        return TValue{};
      }
      if (res == TDecoderResult::FAILURE) {
        result.addError(TError{m_Mode.adapt(TErrorLevel::ERR),
                               fmt::format("{}: value of attribute id={} of component id={} cannot be decoded", context,
                                           attributeId, componentId)});
        return TValue{};
      }
      if (res == TDecoderResult::NO_VALUE) {
        result.addError(TError{
          m_Mode.adapt(TErrorLevel::WARN),
          fmt::format("{}: value of attribute id={} of component id={} is empty", context, attributeId, componentId)});
        return TValue{};
      }
      return value;
    }

    detail::TModeAdapter m_Mode;
    ValueDecoderType m_Decoder;
  };


  class ComponentMapping
  {
  public:
    uint64_t addComponent(uint64_t componentId)
    {
      auto res = ++m_InternalComponentId;
      const auto [_, success] = m_ComponentsMapping.emplace(componentId, res);
      if (!success) {
        throw TException{fmt::format("component id={} already added", componentId)};
      }
      return res;
    }

    inline const TComponent* getComponent(uint64_t referenceId, const rexsapi::TComponents& components) const& noexcept
    {
      const auto it = m_ComponentsMapping.find(referenceId);
      if (it == m_ComponentsMapping.end()) {
        return nullptr;
      }
      const auto it_comp = std::find_if(components.begin(), components.end(), [&it](const auto& comp) {
        return comp.getInternalId() == it->second;
      });
      return it_comp.operator->();
    }

  private:
    // TODO: is this ok?
    inline static uint64_t m_InternalComponentId{0};
    std::unordered_map<uint64_t, uint64_t> m_ComponentsMapping;
  };


  class ComponentPostProcessor
  {
  public:
    ComponentPostProcessor(TResult& result, const detail::TModeAdapter& mode, const rexsapi::TComponents& components,
                           const ComponentMapping& componentMapping)
    {
      process(result, mode, components, componentMapping);
    }

    rexsapi::TComponents&& release() noexcept
    {
      return std::move(m_Components);
    }

  private:
    void process(TResult& result, const detail::TModeAdapter& mode, const rexsapi::TComponents& components,
                 const ComponentMapping& componentMapping) noexcept
    {
      for (const auto& component : components) {
        TAttributes attributes;
        for (const auto& attribute : component.getAttributes()) {
          // TODO: maybe add new value type REFERENCE_EXTERNAL_COMPONENT
          if (attribute.getValueType() == TValueType::REFERENCE_COMPONENT && attribute.hasValue() &&
              attribute.getAttributeId() != "referenced_component_id") {
            auto id = attribute.getValue<TReferenceComponentType>();
            const auto* comp = componentMapping.getComponent(static_cast<uint64_t>(id), components);
            if (comp == nullptr) {
              result.addError(TError{mode.adapt(TErrorLevel::ERR),
                                     fmt::format("referenced component id={} does not exist in component id={}", id,
                                                 component.getExternalId())});
              continue;
            }
            attributes.emplace_back(TAttribute{attribute, TValue{static_cast<int64_t>(comp->getInternalId())}});
          } else {
            attributes.emplace_back(attribute);
          }
        }

        m_Components.emplace_back(TComponent{component, std::move(attributes)});
      }
    }

    rexsapi::TComponents m_Components;
  };
}

#endif
