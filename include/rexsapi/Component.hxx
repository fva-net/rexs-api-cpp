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

#ifndef REXSAPI_COMPONENT_HXX
#define REXSAPI_COMPONENT_HXX

#include <rexsapi/Attribute.hxx>
#include <rexsapi/database/Component.hxx>

namespace rexsapi
{
  /**
   * @brief Represents a REXS model component
   *
   * Components should not be created manually but by using the TModelBuilder.
   * Aggregates attributes and is referenced by relations and load spectrum.
   */
  class TComponent
  {
  public:
    /**
     * @brief Constructs a new TComponent object
     *
     * Components are immutable objects, once created they cannot be changed.
     *
     * @param internalId Has to be unique for this specific component in a specific TModel instance
     * @param componentType The database component type
     * @param name An optional name for this component. Can be left empty.
     * @param attributes The attributes used by this component. Can be left empty. If filled should match the REXS
     * database model attributes allowed for this component.
     */
    TComponent(uint64_t internalId, const database::TComponent& componentType, std::string name, TAttributes attributes)
    : m_InternalId{internalId}
    , m_Type{componentType.getComponentId()}
    , m_Name{std::move(name)}
    , m_Attributes{std::move(attributes)}
    {
    }

    /**
     * @brief Constructs a new TComponent object from an existing component
     *
     * Will use the internal id, type, and name from the existing component.
     *
     * Components are immutable objects, once created they cannot be changed.
     *
     * @param component The component to copy
     * @param attributes The new set of attributes for the new component
     */
    TComponent(const TComponent& component, TAttributes attributes)
    : m_InternalId{component.getInternalId()}
    , m_Type{component.getType()}
    , m_Name{component.getName()}
    , m_Attributes{std::move(attributes)}
    {
    }

    uint64_t getInternalId() const noexcept
    {
      return m_InternalId;
    }

    const std::string& getType() const& noexcept
    {
      return m_Type;
    }

    const std::string& getName() const& noexcept
    {
      return m_Name;
    }

    const TAttributes& getAttributes() const& noexcept
    {
      return m_Attributes;
    }

  private:
    uint64_t m_InternalId;
    std::string m_Type;
    std::string m_Name;
    TAttributes m_Attributes;
  };

  using TComponents = std::vector<TComponent>;
}

#endif
