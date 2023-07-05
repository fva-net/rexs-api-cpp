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

#ifndef REXSAPI_RELATION_HXX
#define REXSAPI_RELATION_HXX

#include <rexsapi/Component.hxx>
#include <rexsapi/Types.hxx>

namespace rexsapi
{
  /**
   * @brief Represents a REXS model relation reference.
   *
   * A relation reference lives in the context of a TRelation and references a component of a model containing the
   * relation.
   *
   * References should not be created manually but by using the TModelBuilder.
   */
  class TRelationReference
  {
  public:
    /**
     * @brief Constructs a new TRelationReference object.
     *
     * References are immutable objects, once created they cannot be changed.
     *
     * @param role The role of the component in the relation
     * @param hint An optional descriptional hint
     * @param component The actual component referenced by this relation reference
     */
    TRelationReference(TRelationRole role, std::string hint, const TComponent& component)
    : m_Role{role}
    , m_Hint{std::move(hint)}
    , m_Component{component}
    {
    }

    [[nodiscard]] const std::string& getHint() const& noexcept
    {
      return m_Hint;
    }

    [[nodiscard]] TRelationRole getRole() const noexcept
    {
      return m_Role;
    }

    [[nodiscard]] const TComponent& getComponent() const& noexcept
    {
      return m_Component;
    }

  private:
    TRelationRole m_Role;
    std::string m_Hint;
    const TComponent& m_Component;
  };

  using TRelationReferences = std::vector<TRelationReference>;


  /**
   * @brief Represents a REXS model relation.
   *
   * A relation aggregates references connecting components to assemblies.
   *
   * Relations should not be created manually but by using the TModelBuilder.
   */
  class TRelation
  {
  public:
    /**
     * @brief Constructs a new TRelation object.
     *
     * Relations are immutable objects, once created they cannot be changed.
     *
     * @param type The type of relation
     * @param order An optional order for the relation. Should be >= 1.
     * @param references The references for this relation
     */
    TRelation(TRelationType type, std::optional<uint32_t> order, TRelationReferences references)
    : m_Type{type}
    , m_Order{std::move(order)}
    , m_References{std::move(references)}
    {
    }

    [[nodiscard]] TRelationType getType() const noexcept
    {
      return m_Type;
    }

    [[nodiscard]] std::optional<uint32_t> getOrder() const noexcept
    {
      return m_Order;
    }

    [[nodiscard]] const TRelationReferences& getReferences() const& noexcept
    {
      return m_References;
    }

  private:
    TRelationType m_Type;
    std::optional<uint32_t> m_Order;
    TRelationReferences m_References;
  };

  using TRelations = std::vector<TRelation>;
}

#endif
