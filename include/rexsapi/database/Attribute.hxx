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

#ifndef REXSAPI_DATABASE_ATTRIBUTE_HXX
#define REXSAPI_DATABASE_ATTRIBUTE_HXX

#include <rexsapi/Format.hxx>
#include <rexsapi/Types.hxx>
#include <rexsapi/database/EnumValues.hxx>
#include <rexsapi/database/Interval.hxx>
#include <rexsapi/database/Unit.hxx>

#include <optional>

/** @file */

namespace rexsapi::database
{
  /**
   * @brief Represents an attribute of a specific REXS database model version.
   *
   * Attributes should not be created manually, but imported from the REXS database model using the
   * TModelRegistry.
   *
   * Each attribute corresponds to an attribute loaded from a specific REXS database model version.
   *
   * An attribute will have a language dependant name (description) that will correspond to the REXS database model
   * language. The language only has an effect on the name of an attribute.
   */
  class TAttribute
  {
  public:
    /**
     * @brief Constructs a new TAttribute object.
     *
     * Attributes are immutable objects, once created they cannot be changed.
     *
     * @param attributeId The attributeId is unique for all attributes of a specific REXS database model version
     * @param name The name is specific to the REXS datbase model language
     * @param type Corresponds to one of the REXS datatypes
     * @param unit The unit has to be one of the REXS database model units
     * @param symbol Will be empty if not set in the REXS database model
     * @param interval Corresponds to the value_range of an attribute
     * @param enumValues Contains all values of the enum. Only set if the attribute is of the TValueType::ENUM type.
     */
    TAttribute(std::string attributeId, std::string name, TValueType type, TUnit unit, std::string symbol,
               std::optional<const TInterval> interval, std::optional<const TEnumValues> enumValues)
    : m_AttributeId{std::move(attributeId)}
    , m_Name{std::move(name)}
    , m_Type{type}
    , m_Unit{std::move(unit)}
    , m_Symbol{std::move(symbol)}
    , m_Interval{std::move(interval)}
    , m_EnumValues{std::move(enumValues)}
    {
      if (m_Type == TValueType::ENUM && !m_EnumValues) {
        throw TException{fmt::format("enum of attribute id={} does not have any values", m_AttributeId)};
      }
    }

    ~TAttribute() = default;

    TAttribute(const TAttribute&) = delete;
    TAttribute& operator=(const TAttribute&) = delete;
    TAttribute(TAttribute&&) = default;
    TAttribute& operator=(TAttribute&&) = delete;

    [[nodiscard]] const std::string& getAttributeId() const& noexcept
    {
      return m_AttributeId;
    }

    [[nodiscard]] const std::string& getName() const& noexcept
    {
      return m_Name;
    }

    [[nodiscard]] TValueType getValueType() const& noexcept
    {
      return m_Type;
    }

    [[nodiscard]] const TUnit& getUnit() const& noexcept
    {
      return m_Unit;
    }

    [[nodiscard]] const std::string& getSymbol() const& noexcept
    {
      return m_Symbol;
    }

    [[nodiscard]] const std::optional<const TInterval>& getInterval() const& noexcept
    {
      return m_Interval;
    }

    [[nodiscard]] const std::optional<const TEnumValues>& getEnums() const& noexcept
    {
      return m_EnumValues;
    }

  private:
    std::string m_AttributeId;
    std::string m_Name;
    TValueType m_Type;
    TUnit m_Unit;
    std::string m_Symbol;
    std::optional<const TInterval> m_Interval;
    std::optional<const TEnumValues> m_EnumValues;
  };
}

#endif
