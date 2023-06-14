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

#ifndef REXSAPI_ATTRIBUTE_HXX
#define REXSAPI_ATTRIBUTE_HXX

#include <rexsapi/Unit.hxx>
#include <rexsapi/Value.hxx>
#include <rexsapi/database/Attribute.hxx>

namespace rexsapi
{
  /**
   * @brief Represents a REXS model attribute
   *
   * An attribute can be either a standard or a custom attribute. Attributes should not be created manually but by using
   * the TModelBuilder.
   *
   * A standard attribute has all the properties of the corresponding REXS database model attribute.
   *
   * A custom attribute should always have the prefix *custom_*. Custom attributes also need an explicit unit and value
   * type.
   */
  class TAttribute
  {
  public:
    /**
     * @brief Constructs a new standard TAttribute object
     *
     * Attributes are immutable objects, once created they cannot be changed.
     *
     * @param attribute A reference to the corresponding database attribute
     * @param value The value for this attribute. Should match the corresponding database attributes TValueType and be
     * in the range specified if applicable.
     */
    TAttribute(const database::TAttribute& attribute, TValue value)
    : m_AttributeWrapper{AttributeWrapper{attribute}}
    , m_Unit{attribute.getUnit()}
    , m_Value{std::move(value)}
    {
    }

    /**
     * @brief Constructs a new custom TAttribute object
     *
     * Attributes are immutable objects, once created they cannot be changed.
     *
     * @param attributeId The custom attribute id. Should always start with the prefix *custom_*.
     * @param unit The unit of this attribute. Is allowed to be a custom unit.
     * @param type The value type of this attribute. Should match the value.
     * @param value The value of this attribute. Should match the value type.
     * @throws TException if the attributeId is empty
     */
    TAttribute(std::string attributeId, TUnit unit, TValueType type, TValue value)
    : m_CustomAttributeId{std::move(attributeId)}
    , m_CustomValueType{type}
    , m_Unit{std::move(unit)}
    , m_Value{std::move(value)}
    {
      if (m_CustomAttributeId.empty()) {
        throw TException{"a custom value is not allowed to have an empty id"};
      }
    }

    /**
     * @brief Constructs a new TAttribute object from an existing attribute
     *
     * Attributes are immutable objects, once created they cannot be changed.
     *
     * @param attribute The attribute to copy
     * @param value The value of the new attribute. Should match the attributes value type and be in the range specified
     * if applicable.
     */
    TAttribute(const TAttribute& attribute, TValue value)
    : m_AttributeWrapper{attribute.m_AttributeWrapper}
    , m_CustomAttributeId{attribute.m_CustomAttributeId}
    , m_CustomValueType{attribute.m_CustomValueType}
    , m_Unit{attribute.m_Unit}
    , m_Value{std::move(value)}
    {
    }

    /**
     * @brief Checks if this attribute is a custom attribute
     *
     * @return true if this attribute is a custom attribute
     * @return false if this attribute is a standard attribute
     */
    [[nodiscard]] bool isCustomAttribute() const noexcept
    {
      return !m_AttributeWrapper.has_value();
    }

    [[nodiscard]] const std::string& getAttributeId() const& noexcept
    {
      if (m_AttributeWrapper) {
        return m_AttributeWrapper->m_Attribute.getAttributeId();
      }
      return m_CustomAttributeId;
    }

    [[nodiscard]] const std::string& getName() const& noexcept
    {
      if (m_AttributeWrapper) {
        return m_AttributeWrapper->m_Attribute.getName();
      }
      return m_CustomAttributeId;
    }

    [[nodiscard]] const TUnit& getUnit() const& noexcept
    {
      return m_Unit;
    }

    [[nodiscard]] TValueType getValueType() const noexcept
    {
      if (m_AttributeWrapper) {
        return m_AttributeWrapper->m_Attribute.getValueType();
      }
      return *m_CustomValueType;
    }

    /**
     * @brief Checks if this attribute has a non empty value
     *
     * An empty value can be created by passing a default TValue instance into a constructor.
     *
     * @return true if value is not empty
     * @return false if value is empty
     */
    [[nodiscard]] bool hasValue() const noexcept
    {
      return !m_Value.isEmpty();
    }

    [[nodiscard]] const TValue& getValue() const& noexcept
    {
      return m_Value;
    }

    /**
     * @brief Returns the underlying C++ value
     *
     * @tparam T The actual C++ type to extract. Should use one of the predefined types: TFloatType, TBoolType,
     * TIntType, TEnumType, TStringType, TFileReferenceType, TBoolArrayType, TFloatArrayType, TIntArrayType,
     * TEnumArrayType, TStringArrayType, TDatetimeType, TReferenceComponentType, TFloatMatrixType, TStringMatrixType,
     * TArrayOfIntArraysType.
     * @return const auto& to the underlying C++ value
     * @throws std::bad_variant_access if T does not correspond to the underlying type
     */
    template<typename T>
    [[nodiscard]] const auto& getValue() const&
    {
      return m_Value.getValue<T>();
    }

    /**
     * @brief Returns the value as string
     *
     * Currently, will not convert array, array of arrays or matrix types
     *
     * @return std::string of the value
     * @throws TException if non convertible type
     */
    [[nodiscard]] std::string getValueAsString() const
    {
      return m_Value.asString();
    }

  private:
    struct AttributeWrapper {
      const database::TAttribute& m_Attribute;
    };
    std::optional<AttributeWrapper> m_AttributeWrapper;

    std::string m_CustomAttributeId{};
    std::optional<TValueType> m_CustomValueType{};

    TUnit m_Unit;
    TValue m_Value;
  };

  using TAttributes = std::vector<TAttribute>;
}

#endif
