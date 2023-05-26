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

#ifndef REXSAPI_VALIDITY_CHECKER_HXX
#define REXSAPI_VALIDITY_CHECKER_HXX

#include <rexsapi/Attribute.hxx>


namespace rexsapi::detail
{
  class TValidityChecker
  {
  public:
    static bool check(const database::TAttribute& attribute, const TValue& val);
  };


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  static inline bool checkRange(const std::optional<const database::TInterval>& interval, double val)
  {
    if (interval.has_value()) {
      return interval->check(val);
    }
    return true;
  }

  inline bool TValidityChecker::check(const database::TAttribute& attribute, const TValue& val)
  {
    const auto& interval = attribute.getInterval();

    switch (attribute.getValueType()) {
      case TValueType::FLOATING_POINT:
        return checkRange(interval, val.getValue<TFloatType>());
      case TValueType::INTEGER:
        return checkRange(interval, static_cast<double>(val.getValue<TIntType>()));
      case TValueType::ENUM:
        return attribute.getEnums()->check(val.getValue<TEnumType>());
      case TValueType::ENUM_ARRAY: {
        const auto& enums = attribute.getEnums();
        const auto& values = val.getValue<TEnumArrayType>();
        return std::all_of(values.begin(), values.end(), [&enums](const auto& enum_val) {
          return enums->check(enum_val);
        });
      }
      case TValueType::FLOATING_POINT_ARRAY: {
        const auto& values = val.getValue<TFloatArrayType>();
        return std::all_of(values.begin(), values.end(), [&interval](const auto& d) {
          return checkRange(interval, d);
        });
      }
      case TValueType::INTEGER_ARRAY: {
        const auto& values = val.getValue<TIntArrayType>();
        return std::all_of(values.begin(), values.end(), [&interval](const auto& i) {
          return checkRange(interval, static_cast<double>(i));
        });
      }
      case TValueType::ARRAY_OF_INTEGER_ARRAYS: {
        const auto& values = val.getValue<TArrayOfIntArraysType>();
        return std::all_of(values.begin(), values.end(), [&interval](const auto& v) {
          return std::all_of(v.begin(), v.end(), [&interval](const auto& i) {
            return checkRange(interval, static_cast<double>(i));
          });
        });
      }
      case TValueType::FLOATING_POINT_MATRIX: {
        const auto& values = val.getValue<TFloatMatrixType>().m_Values;
        return std::all_of(values.begin(), values.end(), [&interval](const auto& v) {
          return std::all_of(v.begin(), v.end(), [&interval](const auto& d) {
            return checkRange(interval, d);
          });
        });
      }
      case TValueType::INTEGER_MATRIX: {
        const auto& values = val.getValue<TIntMatrixType>().m_Values;
        return std::all_of(values.begin(), values.end(), [&interval](const auto& v) {
          return std::all_of(v.begin(), v.end(), [&interval](const auto& i) {
            return checkRange(interval, static_cast<double>(i));
          });
        });
      }
      case TValueType::BOOLEAN:
      case TValueType::STRING:
      case TValueType::FILE_REFERENCE:
      case TValueType::BOOLEAN_ARRAY:
      case TValueType::STRING_ARRAY:
      case TValueType::BOOLEAN_MATRIX:
      case TValueType::STRING_MATRIX:
      case TValueType::REFERENCE_COMPONENT:
      case TValueType::DATE_TIME:
        return true;
    }

    return true;
  }
}

#endif
