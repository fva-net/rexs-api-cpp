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

#ifndef REXSAPI_DATABASE_ENUM_VALUES
#define REXSAPI_DATABASE_ENUM_VALUES

#include <algorithm>
#include <string>
#include <vector>

/** @file */

namespace rexsapi::database
{
  /**
   * @brief Represents a single enum value of an enum.
   *
   */
  struct TEnumValue {
    std::string m_Value;  //!< Represents the enums value
    std::string m_Name;   //!< The name is specific to the REXS datbase model language
  };


  /**
   * @brief Represents the enum values of an attribute.
   *
   * Enum values should not be created manually, but imported from the REXS database model using the
   * TModelRegistry.
   *
   * Enum values correspond to enum values loaded alongside their attributes from a specific REXS database
   * model version.
   */
  class TEnumValues
  {
  public:
    /**
     * @brief Constructs a new TEnumValues object.
     *
     * Enum values are immutable objects, once created they cannot be changed.
     *
     * @param values All enum values defined for a specific attribute
     */

    explicit TEnumValues(std::vector<TEnumValue>&& values)
    : m_Values{std::move(values)}
    {
    }

    /**
     * @brief Checks if a given value is contained in this enumeration.
     *
     * @param value The value to check this enumeration for
     * @return true if the value is contained
     * @return false if the value is not contained
     */
    [[nodiscard]] bool check(const std::string& value) const noexcept;

  private:
    std::vector<TEnumValue> m_Values;
  };


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  inline bool TEnumValues::check(const std::string& value) const noexcept
  {
    auto it = std::find_if(m_Values.begin(), m_Values.end(), [&value](const auto& entry) {
      return entry.m_Value == value;
    });

    return it != m_Values.end();
  }
}

#endif
