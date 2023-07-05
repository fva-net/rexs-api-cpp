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

#ifndef REXSAPI_DATABASE_COMPONENT_HXX
#define REXSAPI_DATABASE_COMPONENT_HXX

#include <rexsapi/database/Attribute.hxx>

/** @file */

namespace rexsapi::database
{
  /**
   * @brief Represents a component of a specific REXS database model version.
   *
   * Components should not be created manually, but imported from the REXS database model using the
   * TModelRegistry.
   *
   * Each component corresponds to a component loaded from a specific REXS database model version.
   *
   * Each component of a REXS model has some attributes associated to it and a REXS model is only allowed to use these
   * associated attributes in order to be standard compliant.
   */
  class TComponent
  {
  public:
    /**
     * @brief Constructs a new TComponent object.
     *
     * Components are immutable objects, once created they cannot be changed.
     *
     * @param componentId The componentId is unique for all components of a specific REXS database model version
     * @param name The name is specific to the REXS datbase model language
     * @param attributes All attributes associated to this component
     */
    TComponent(std::string componentId, std::string name,
               std::vector<std::reference_wrapper<const TAttribute>>&& attributes)
    : m_ComponentId{std::move(componentId)}
    , m_Name{std::move(name)}
    , m_Attributes{std::move(attributes)}
    {
    }

    ~TComponent() = default;

    TComponent(const TComponent&) = delete;
    TComponent& operator=(const TComponent&) = delete;
    TComponent(TComponent&&) noexcept = default;
    TComponent& operator=(TComponent&&) = delete;

    [[nodiscard]] const std::string& getComponentId() const& noexcept
    {
      return m_ComponentId;
    }

    [[nodiscard]] const std::string& getName() const& noexcept
    {
      return m_Name;
    }

    [[nodiscard]] std::vector<std::reference_wrapper<const TAttribute>> getAttributes() const noexcept
    {
      return m_Attributes;
    }

    /**
     * @brief Checks if the component contains an attribute with the given attributeId.
     *
     * Attributes not contained in this component are not allowed to be associated to a REXS model component.
     *
     * @param attributeId
     * @return true if the attribute with the attributeId is present
     * @return false if no attribute with the attributeId is present
     */
    [[nodiscard]] bool hasAttribute(const std::string& attributeId) const& noexcept
    {
      auto it = std::find_if(m_Attributes.begin(), m_Attributes.end(), [&attributeId](const TAttribute& attribute) {
        return attribute.getAttributeId() == attributeId;
      });
      return it != m_Attributes.end();
    }

    /**
     * @brief Retrieves an attribute with the given attributeId.
     *
     * @param attributeId The attributeId of the attribute to retrieve
     * @return const TAttribute& to the found attribute
     * @throws TException if no attribute with the given attributeId is found in the component
     */
    [[nodiscard]] const TAttribute& findAttributeById(const std::string& attributeId) const&
    {
      auto it = std::find_if(m_Attributes.begin(), m_Attributes.end(), [&attributeId](const TAttribute& attribute) {
        return attribute.getAttributeId() == attributeId;
      });
      if (it == m_Attributes.end()) {
        throw TException{fmt::format("component id={} does not contain attribute id={}", m_ComponentId, attributeId)};
      }

      return *it;
    }

  private:
    std::string m_ComponentId;
    std::string m_Name;
    std::vector<std::reference_wrapper<const TAttribute>> m_Attributes;
  };
}

#endif
