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

#ifndef REXSAPI_UNIT_HXX
#define REXSAPI_UNIT_HXX

#include <rexsapi/database/Unit.hxx>

#include <cstdint>


namespace rexsapi
{
  /**
   * @brief Represents a REXS model unit.
   *
   * Represents either a standard REXS unit or a custom unit.
   *
   */
  class TUnit
  {
  public:
    /**
     * @brief Constructs a new empty custom TUnit object.
     *
     */
    TUnit() = default;

    /**
     * @brief Constructs a new TUnit object from a REXS database unit.
     *
     * The new unit will have the same name as the database unit.
     *
     * @param unit The unit to create this unit with
     */
    explicit TUnit(const database::TUnit& unit)
    : m_Unit{unit.getName()}
    , m_IsCustomUnit{false}
    {
    }

    /**
     * @brief Constructs a new custom TUnit object.
     *
     * @param unit The units name
     */
    explicit TUnit(std::string unit)
    : m_Unit{std::move(unit)}
    {
    }

    /**
     * @brief Checks if this unit is a custom unit.
     *
     * @return true if constructed as a default unit or as a custom unit
     * @return false if constructed with a REXS database unit
     */
    [[nodiscard]] inline bool isCustomUnit() const noexcept
    {
      return m_IsCustomUnit;
    }

    [[nodiscard]] const std::string& getName() const& noexcept
    {
      return m_Unit;
    }

    /**
     * @brief Compares two units for equality.
     *
     * @param lhs A REXS model unit
     * @param rhs A REXS database unit
     * @return true if the name is the same for both units
     * @return false if the units have differing names
     */
    friend bool operator==(const TUnit& lhs, const database::TUnit& rhs)
    {
      return rhs.compare(lhs.m_Unit);
    }

    /**
     * @brief Compares two units for inequality.
     *
     * @param lhs A REXS model unit
     * @param rhs A REXS database unit
     * @return true if the units have differing names
     * @return false if both units have the same name
     */
    friend bool operator!=(const TUnit& lhs, const database::TUnit& rhs) noexcept
    {
      return !(lhs == rhs);
    }

    /**
     * @brief Compares two REXS model units for equality.
     *
     * @param lhs The left hand unit to compare
     * @param rhs The right hand unit to compare
     * @return true if both units have the same name
     * @return false if the units have differing names
     */
    friend bool operator==(const TUnit& lhs, const TUnit& rhs) noexcept
    {
      return lhs.m_Unit == rhs.m_Unit;
    }

    /**
     * @brief Compares two REXS model units for inequality.
     *
     * @param lhs The left hand unit to compare
     * @param rhs The right hand unit to compare
     * @return true if the units have differing names
     * @return false if both units have the same name
     */
    friend bool operator!=(const TUnit& lhs, const TUnit& rhs) noexcept
    {
      return !(lhs == rhs);
    }

  private:
    std::string m_Unit{};
    bool m_IsCustomUnit{true};
  };
}

#endif
