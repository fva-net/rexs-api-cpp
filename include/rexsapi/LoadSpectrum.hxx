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

#ifndef REXSAPI_LOAD_SPECTRUM_HXX
#define REXSAPI_LOAD_SPECTRUM_HXX

#include <rexsapi/Component.hxx>

namespace rexsapi
{
  /**
   * @brief Represents a component in a load case or accumulation.
   *
   * References an existing component and adds additional load attributes to it.
   *
   * Load components should not be created manually but by using the TModelBuilder.
   */
  class TLoadComponent
  {
  public:
    /**
     * @brief Constructs a new TLoadComponent object.
     *
     * Load components are immutable objects, once created they cannot be changed.
     *
     * @param component The referenced component
     * @param attributes Additonal attributes for the load case. Can be left empty. Should not contain the same
     * attributes as the referenced component.
     */
    TLoadComponent(const TComponent& component, TAttributes attributes)
    : m_Component{component}
    , m_Attributes{attributes}
    , m_LoadAttributes{std::move(attributes)}
    {
      std::for_each(m_Component.getAttributes().begin(), m_Component.getAttributes().end(),
                    [this](const auto& attribute) {
                      m_Attributes.emplace_back(attribute);
                    });
    }

    const TComponent& getComponent() const& noexcept
    {
      return m_Component;
    }

    /**
     * @brief Returns the complete set of attributes including the referenced components.
     *
     * Will combine the referenced components attributes with the additional load case attributes.
     *
     * @return const TAttributes& to the complete attributes
     */
    const TAttributes& getAttributes() const& noexcept
    {
      return m_Attributes;
    }

    /**
     * @brief Returns exclusively the additional load attributes.
     *
     * Will not return the referenced components attributes.
     *
     * @return const TAttributes& to the additional load attributes
     */
    const TAttributes& getLoadAttributes() const& noexcept
    {
      return m_LoadAttributes;
    }

  private:
    const TComponent& m_Component;
    TAttributes m_Attributes;
    TAttributes m_LoadAttributes;
  };

  using TLoadComponents = std::vector<TLoadComponent>;


  /**
   * @brief Represents a single load spectrum load case.
   *
   * Aggregates load case components with load attributes.
   *
   * Load cases are immutable objects, once created they cannot be changed.
   *
   * Load cases should not be created manually but by using the TModelBuilder.
   */
  class TLoadCase
  {
  public:
    explicit TLoadCase(TLoadComponents components)
    : m_Components{std::move(components)}
    {
    }

    const TLoadComponents& getLoadComponents() const& noexcept
    {
      return m_Components;
    }

  private:
    TLoadComponents m_Components;
  };

  using TLoadCases = std::vector<TLoadCase>;


  /**
   * @brief Represents the load spectrum accumulation.
   *
   * Aggregates accumulation components with load attributes.
   *
   * Accumulations are immutable objects, once created they cannot be changed.
   *
   * Accumulations should not be created manually but by using the TModelBuilder.
   */
  class TAccumulation
  {
  public:
    explicit TAccumulation(TLoadComponents components)
    : m_Components{std::move(components)}
    {
    }

    const TLoadComponents& getLoadComponents() const& noexcept
    {
      return m_Components;
    }

  private:
    TLoadComponents m_Components;
  };


  /**
   * @brief Represents the load spectrum containing load cases and an optional accumulation.
   *
   * A load spectrum should not be created manually but by using the TModelBuilder.
   */
  class TLoadSpectrum
  {
  public:
    /**
     * @brief Constructs a new TLoadSpectrum object.
     *
     * A load spectrum can contain a number of load cases and one optional accumulation.
     *
     * Load spectra are immutable objects, once created they cannot be changed.
     *
     * @param loadCases All load cases of this load spectrum. Can be left empty if the load spectrum is not to be
     * configured.
     * @param accumulation An optinal accumulation
     */
    explicit TLoadSpectrum(TLoadCases loadCases, std::optional<TAccumulation> accumulation)
    : m_LoadCases{std::move(loadCases)}
    , m_Accumulation{std::move(accumulation)}
    {
    }

    /**
     * @brief Check of this load spectrum contains any load cases.
     *
     * @return true if there are load cases
     * @return false if no load cases are available
     */
    [[nodiscard]] bool hasLoadCases() const noexcept
    {
      return !m_LoadCases.empty();
    }

    const TLoadCases& getLoadCases() const& noexcept
    {
      return m_LoadCases;
    }

    /**
     * @brief Check of this load spectrum contains an accumulation.
     *
     * @return true if there is an accumulation available
     * @return false if no accumulation is available
     */
    [[nodiscard]] bool hasAccumulation() const noexcept
    {
      return m_Accumulation.has_value();
    }

    /**
     * @brief Returns the accumulation if available.
     *
     * @return const TAccumulation& to the accumulation
     * @throws TException if there is no accumulation available
     */
    const TAccumulation& getAccumulation() const&
    {
      if (m_Accumulation.has_value()) {
        return m_Accumulation.value();
      }
      throw TException{"model does not have an accumulation"};
    }

  private:
    TLoadCases m_LoadCases;
    std::optional<TAccumulation> m_Accumulation;
  };
}

#endif
