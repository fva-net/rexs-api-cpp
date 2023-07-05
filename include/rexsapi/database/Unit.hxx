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

#ifndef REXSAPI_DATABASE_UNIT_HXX
#define REXSAPI_DATABASE_UNIT_HXX

#include <string>

/** @file */

namespace rexsapi::database
{
  /**
   * @brief Represents a unit of a specific REXS database model version.
   *
   * Units should not be created manually, but imported from the REXS database model using the
   * TModelRegistry.
   *
   * Each unit corresponds to a unit loaded from a specific REXS database model version.
   */
  class TUnit
  {
  public:
    /**
     * @brief Constructs a new TUnit object.
     *
     * Units are immutable objects, once created they cannot be changed.
     *
     * @param id A unique id for the unit
     * @param name A unique name for the unit
     */
    TUnit(uint64_t id, std::string name)
    : m_Id{id}
    , m_Name{std::move(name)}
    {
    }

    [[nodiscard]] uint64_t getId() const noexcept
    {
      return m_Id;
    }

    [[nodiscard]] const std::string& getName() const noexcept
    {
      return m_Name;
    }

    [[nodiscard]] bool compare(std::string_view name) const noexcept
    {
      return m_Name == name;
    }

    /**
     * @brief Compares two units for equality.
     *
     * @param lhs The left hand unit to compare
     * @param rhs The right hand unit to compare
     * @return true if both units have the same name
     * @return false if the units have differing names
     */
    friend bool operator==(const TUnit& lhs, const TUnit& rhs) noexcept
    {
      return lhs.compare(rhs.getName());
    }

    /**
     * @brief Compares two units for inequality.
     *
     * @param lhs The left hand unit to compare
     * @param rhs The right hand unit to compare
     * @return true if the units have differing names
     * @return false if both units have the same name
     */
    friend bool operator!=(const TUnit& lhs, const TUnit& rhs) noexcept
    {
      return !lhs.compare(rhs.getName());
    }

  private:
    uint64_t m_Id{};
    std::string m_Name{};
  };
}

#endif
