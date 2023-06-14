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
     * @attention The external id will be set to uint64 max
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
     * @brief Constructs a new TComponent object with an external component id
     *
     * Components are immutable objects, once created they cannot be changed.
     *
     * @param externalId External id specified in a rexs model file. Has to be unique for this specific component in a specific TModel instance.
     * @param internalId Internal id used for referencing. Has to be unique for this specific component in a specific TModel instance.
     * @param componentType The database component type.
     * @param name An optional name for this component. Can be left empty.
     * @param attributes The attributes used by this component. Can be left empty. If filled should match the REXS
     * database model attributes allowed for this component.
     */
    TComponent(uint64_t externalId, uint64_t internalId, const database::TComponent& componentType, std::string name,
               TAttributes attributes)
    : m_ExternalId{externalId}
    , m_InternalId{internalId}
    , m_Type{componentType.getComponentId()}
    , m_Name{std::move(name)}
    , m_Attributes{std::move(attributes)}
    {
    }

    /**
     * @brief Constructs a new TComponent object from an existing component
     *
     * Will use the external id, internal id, type, and name from the existing component.
     *
     * Components are immutable objects, once created they cannot be changed.
     *
     * @param component The component to copy
     * @param attributes The new set of attributes for the new component
     */
    TComponent(const TComponent& component, TAttributes attributes)
    : m_ExternalId{component.getExternalId()}
    , m_InternalId{component.getInternalId()}
    , m_Type{component.getType()}
    , m_Name{component.getName()}
    , m_Attributes{std::move(attributes)}
    {
    }

    /**
     * @brief Returns the external id (e.g from a model file) of this component
     * 
     * @attention The external id will return uint64 max, if it has not been set
     * 
     * @return The external component id
     */ 
    uint64_t getExternalId() const noexcept
    {
      return m_ExternalId;
    }

    /**
     * @brief Returns the internal id of this component
     *
     * The internal id will either be set manually by the user or generated automatically by the API.
     *
     * @return The internal component id
     */
    uint64_t getInternalId() const noexcept
    {
      return m_InternalId;
    }

    /**
     * @brief Returns either the internal or external id of this component
     *
     * Will return the external id if it has been set. Otherwise, returns the internal id.
     *
     * @return The components id
     */
    uint64_t getId() const noexcept
    {
      return m_ExternalId != std::numeric_limits<uint64_t>::max() ? m_ExternalId : m_InternalId;
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
    uint64_t m_ExternalId{std::numeric_limits<uint64_t>::max()};
    uint64_t m_InternalId;
    std::string m_Type;
    std::string m_Name;
    TAttributes m_Attributes;
  };

  using TComponents = std::vector<TComponent>;
}

#endif
