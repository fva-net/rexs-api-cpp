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

#ifndef REXSAPI_VALUE_HXX
#define REXSAPI_VALUE_HXX

#include <rexsapi/ConversionHelper.hxx>
#include <rexsapi/Types.hxx>
#include <rexsapi/Value_Details.hxx>
#include <rexsapi/database/EnumValues.hxx>

#include <functional>

namespace rexsapi
{
  /**
   * @brief Defines how to encode a value
   *
   * Has only effect on integer and floating point array and matrix values.
   *
   */
  enum class TCodeType {
    None,      //!< No encoding
    Default,   //!< Default encoding for the given value. In case of integer, will encode as int32. In case of floating
               //!< point, will encode as float64.
    Optimized  //!< Optimizes the encoding. In case of floating point, will encode das float32.
  };


  /**
   * @brief Represents a value of a REXS model attribute
   *
   * Each standard compliant REXS model attribute has a value. As each attribute has a concrete value type that can be
   * mapped to a C++ type, the actual value has a concrete underlying type.
   *
   * In general a value is a generic object and can be used in a generic way. However, the actual value can only be
   * retrieved using a concrete C++ type. There are typedefs for each value type to C++ type mapping, that should always
   * be used. For a floating point value type that would be rexsapi::TFloatType, a bool would be rexsapi::TBoolType, and
   * so on.
   */
  class TValue
  {
  public:
    /**
     * @brief Constructs a new empty TValue object
     *
     */
    TValue() = default;

    /**
     * @brief Constructs a new TValue object with a value of a specific C++ type
     *
     * @tparam T The C++ type of the value. Has to be one of the allowed types in order to compile.
     * @param val The actual value to construct this object with
     */
    template<typename T>
    explicit TValue(T&& val)
    : m_Value(std::forward<T>(val))
    {
    }

    /**
     * @brief Constructs a new TValue object with a string
     *
     * Will be of rexsapi::TStringType type.
     *
     * @param val The string to assign to this value
     */
    explicit TValue(const char* val)
    : m_Value(std::string(val))
    {
    }

    /**
     * @brief Constructs a new TValue object with an integer
     *
     * Will be of rexsapi::TIntType type.
     *
     * @param val The integer to assign to this value
     */
    explicit TValue(int val)
    : m_Value(static_cast<int64_t>(val))
    {
    }

    /**
     * @brief Constructs a new TValue object with a boolean
     *
     * Will be of rexsapi::TBoolType type.
     *
     * @param val The boolean to assign to this value
     */
    explicit TValue(Bool val)
    : m_Value(val.m_Value)
    {
    }

    /**
     * @brief Assigns a different value to this object
     *
     * @tparam T The C++ type of the value. Has to be one of the allowed types in order to compile.
     * @param val The actual value to assign to this object
     * @return TValue& to this value
     */
    template<typename T>
    TValue& operator=(T&& val) noexcept
    {
      m_Value = std::forward<T>(val);
      return *this;
    }

    /**
     * @brief Assigns a different value to this object
     *
     * Will be of rexsapi::TIntType type.
     *
     * @param val The actual value to assign to this object
     * @return TValue& to this value
     */
    TValue& operator=(int val)
    {
      m_Value = static_cast<int64_t>(val);
      return *this;
    }

    /**
     * @brief Assigns a different value to this object
     *
     * Will be of rexsapi::TStringType type.
     *
     * @param val The actual value to assign to this object
     * @return TValue& to this value
     */
    TValue& operator=(const char* val)
    {
      m_Value = std::string(val);
      return *this;
    }

    /**
     * @brief Checks if this object has a value assigned
     *
     * @return true default constructed, no value assigned
     * @return false some value assigned
     */
    bool isEmpty() const noexcept
    {
      return m_Value.index() == 0;
    }

    /**
     * @brief Extracts a concrete value type from this object
     *
     * @tparam T The type to extract from this object. Should always use one of the provided typedefs like
     * rexsapi::TFloatType, etc.
     * @return const auto& to the underlying C++ value
     * @throws std::bad_variant_access if the internal type does not correspong to the extracted type
     */
    template<typename T>
    const auto& getValue() const&
    {
      return detail::value_getter<T>(m_Value);
    }

    /**
     * @brief Extracts a concrete typed value from this object or a default value if empty
     *
     * @tparam T The type to extract from this object. Should always use one of the provided typedefs like
     * rexsapi::TFloatType, etc.
     * @param def The default value to use if this value is empty
     * @return const auto& to the underlying C++ value or the default value if empty
     * @throws std::bad_variant_access if the internal type does not correspond to the extracted type
     */
    template<typename T>
    const auto& getValue(const T& def) const&
    {
      if (m_Value.index() == 0) {
        return def;
      }
      return detail::value_getter<T>(m_Value);
    }

    friend bool operator==(const TValue& lhs, const TValue& rhs) noexcept
    {
      // ATTENTION: will currently compare floating point values with ==
      return lhs.m_Value == rhs.m_Value;
    }

    /**
     * @brief Returns a string representation of the underlying C++ value
     *
     * @return std::string representation of the underlying C++ value
     */
    std::string asString() const;

    /**
     * @brief Sets the encoding type for this value
     *
     * Will only be effective for integer or float types. Other types will ignore the setting.
     *
     * @param type The encoding type for this value
     */
    void coded(TCodeType type) noexcept
    {
      m_CodeType = type;
    }

    TCodeType coded() const noexcept
    {
      return m_CodeType;
    }

    /**
     * @brief Checks if the given value type corresponds to the underlying C++ type
     *
     * @param type The type to check
     * @return true the underlying C++ type can be converted to the value type
     * @return false the underlying C++ type connot be converted to the value type
     */
    bool matchesValueType(TValueType type) const noexcept
    {
      switch (type) {
        case TValueType::FLOATING_POINT:
          return std::holds_alternative<TFloatType>(m_Value);
        case TValueType::INTEGER:
          return std::holds_alternative<TIntType>(m_Value);
        case TValueType::BOOLEAN:
          return std::holds_alternative<TBoolType>(m_Value);
        case TValueType::ENUM:
          return std::holds_alternative<TEnumType>(m_Value);
        case TValueType::STRING:
          return std::holds_alternative<TStringType>(m_Value);
        case TValueType::FILE_REFERENCE:
          return std::holds_alternative<TFileReferenceType>(m_Value);
        case TValueType::FLOATING_POINT_ARRAY:
          return std::holds_alternative<TFloatArrayType>(m_Value);
        case TValueType::BOOLEAN_ARRAY:
          return std::holds_alternative<TBoolArrayType>(m_Value);
        case TValueType::INTEGER_ARRAY:
          return std::holds_alternative<TIntArrayType>(m_Value);
        case TValueType::ENUM_ARRAY:
          return std::holds_alternative<TEnumArrayType>(m_Value);
        case TValueType::STRING_ARRAY:
          return std::holds_alternative<TStringArrayType>(m_Value);
        case TValueType::REFERENCE_COMPONENT:
          return std::holds_alternative<TReferenceComponentType>(m_Value);
        case TValueType::FLOATING_POINT_MATRIX:
          return std::holds_alternative<TFloatMatrixType>(m_Value);
        case TValueType::INTEGER_MATRIX:
          return std::holds_alternative<TIntMatrixType>(m_Value);
        case TValueType::BOOLEAN_MATRIX:
          return std::holds_alternative<TBoolMatrixType>(m_Value);
        case TValueType::STRING_MATRIX:
          return std::holds_alternative<TStringMatrixType>(m_Value);
        case TValueType::ARRAY_OF_INTEGER_ARRAYS:
          return std::holds_alternative<TArrayOfIntArraysType>(m_Value);
      }
      return false;
    }

  private:
    detail::Variant m_Value;
    TCodeType m_CodeType{TCodeType::None};
  };


  template<typename R>
  using DispatcherFuncs = std::tuple<
    std::function<R(TFloatTag, const TFloatType&)>, std::function<R(TBoolTag, const bool&)>,
    std::function<R(TIntTag, const TIntType&)>, std::function<R(TEnumTag, const std::string&)>,
    std::function<R(TStringTag, const TStringType&)>, std::function<R(TFileReferenceTag, const TFileReferenceType&)>,
    std::function<R(TFloatArrayTag, const TFloatArrayType&)>, std::function<R(TBoolArrayTag, const TBoolArrayType&)>,
    std::function<R(TIntArrayTag, const TIntArrayType&)>, std::function<R(TEnumArrayTag, const TEnumArrayType&)>,
    std::function<R(TStringArrayTag, const TStringArrayType&)>,
    std::function<R(TReferenceComponentTag, const TReferenceComponentType&)>,
    std::function<R(TFloatMatrixTag, const TFloatMatrixType&)>, std::function<R(TIntMatrixTag, const TIntMatrixType&)>,
    std::function<R(TBoolMatrixTag, const TBoolMatrixType&)>,
    std::function<R(TStringMatrixTag, const TStringMatrixType&)>,
    std::function<R(TArrayOfIntArraysTag, const TArrayOfIntArraysType&)>>;

  /**
   * @brief Dispatches a value to the corresponding function type of the given dispatcher functions
   *
   * Can be used to work on values in a generic way. E.g., a REXS attribute always has a value type and a value. In
   * order to not have multiple long switch statements for value types, a dispatcher function tuple can be created and
   * reused.
   *
   * @tparam R The return type of the operation
   * @param type The value type of the dispatcher function to call
   * @param value The actual value to hand over to the dispatcher function. The values type shall correspond to the
   * value type given.
   * @param funcs The dispatcher function tuple. The functions in the tuple have to have the exact order as defined by
   * the rexsapi::DispatcherFuncs using directive.
   * @return auto Returns a value of type R. If R is void, no value will be returned.
   * @throws std::bad_variant_access if the values type does not correspond to the value type given
   */
  template<typename R>
  auto dispatch(TValueType type, const TValue& value, DispatcherFuncs<R> funcs);


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  namespace detail
  {
    template<typename T>
    std::string arrayToString(const std::vector<T>& array,
                              std::function<std::string(typename std::vector<T>::value_type)>&& formatter)
    {
      std::stringstream stream;
      stream << "[";
      bool first{true};
      for (const T& val : array) {
        if (first) {
          first = false;
        } else {
          stream << ",";
        }
        stream << formatter(val);
      }
      stream << "]";
      return stream.str();
    }

    template<typename T>
    std::string array2dToString(const std::vector<std::vector<T>>& array,
                                std::function<std::string(typename std::vector<T>::value_type)>&& formatter)
    {
      std::stringstream stream;
      stream << "[";
      bool first{true};
      for (const auto& row : array) {
        if (first) {
          first = false;
        } else {
          stream << ",";
        }
        stream << "[";
        bool firstCol{true};
        for (const auto& col : row) {
          if (firstCol) {
            firstCol = false;
          } else {
            stream << ",";
          }
          stream << formatter(col);
        }
        stream << "]";
      }
      stream << "]";
      return stream.str();
    }
  }

  inline std::string TValue::asString() const
  {
    return std::visit(detail::overload{[](const std::monostate&) -> std::string {
                                         return "";
                                       },
                                       [](const std::string& s) -> std::string {
                                         return s;
                                       },
                                       [](const bool& b) -> std::string {
                                         return fmt::format("{}", b);
                                       },
                                       [](const double& d) -> std::string {
                                         return format(d);
                                       },
                                       [](const int64_t& i) -> std::string {
                                         return fmt::format("{}", i);
                                       },
                                       [](const std::vector<double>& array) -> std::string {
                                         return detail::arrayToString(array, [](const auto& val) {
                                           return format(val);
                                         });
                                       },
                                       [](const std::vector<Bool>& array) -> std::string {
                                         return detail::arrayToString(array, [](const auto& val) {
                                           return fmt::format("{}", *val);
                                         });
                                       },
                                       [](const std::vector<int64_t>& array) -> std::string {
                                         return detail::arrayToString(array, [](const auto& val) {
                                           return fmt::format("{}", val);
                                         });
                                       },
                                       [](const std::vector<std::string>& array) -> std::string {
                                         return detail::arrayToString(array, [](const auto& val) {
                                           return fmt::format("{}", val);
                                         });
                                       },
                                       [](const std::vector<std::vector<int64_t>>& array) -> std::string {
                                         return detail::array2dToString(array, [](const auto& val) {
                                           return fmt::format("{}", val);
                                         });
                                       },
                                       [](const TMatrix<double>& matrix) -> std::string {
                                         return detail::array2dToString(matrix.m_Values, [](const auto& val) {
                                           return format(val);
                                         });
                                       },
                                       [](const TMatrix<int64_t>& matrix) -> std::string {
                                         return detail::array2dToString(matrix.m_Values, [](const auto& val) {
                                           return fmt::format("{}", val);
                                         });
                                       },
                                       [](const TMatrix<Bool>& matrix) -> std::string {
                                         return detail::array2dToString(matrix.m_Values, [](const auto& val) {
                                           return fmt::format("{}", *val);
                                         });
                                       },
                                       [](const TMatrix<std::string>& matrix) -> std::string {
                                         return detail::array2dToString(matrix.m_Values, [](const auto& val) {
                                           return fmt::format("{}", val);
                                         });
                                       }},
                      m_Value);
  }

  template<typename R>
  inline auto dispatch(TValueType type, const TValue& value, DispatcherFuncs<R> funcs)
  {
    try {
      switch (type) {
        case TValueType::FLOATING_POINT: {
          auto c = std::get<std::function<R(TFloatTag, const TFloatType&)>>(funcs);
          if (c) {
            return c(TFloatTag(), value.getValue<TFloatType>());
          }
          break;
        }
        case TValueType::BOOLEAN: {
          auto c = std::get<std::function<R(TBoolTag, const bool&)>>(funcs);
          if (c) {
            return c(TBoolTag(), value.getValue<TBoolType>());
          }
          break;
        }
        case TValueType::INTEGER: {
          auto c = std::get<std::function<R(TIntTag, const TIntType&)>>(funcs);
          if (c) {
            return c(TIntTag(), value.getValue<TIntType>());
          }
          break;
        }
        case TValueType::ENUM: {
          auto c = std::get<std::function<R(TEnumTag, const TEnumType&)>>(funcs);
          if (c) {
            return c(TEnumTag(), value.getValue<TEnumType>());
          }
          break;
        }
        case TValueType::STRING: {
          auto c = std::get<std::function<R(TStringTag, const TStringType&)>>(funcs);
          if (c) {
            return c(TStringTag(), value.getValue<TStringType>());
          }
          break;
        }
        case TValueType::FILE_REFERENCE: {
          auto c = std::get<std::function<R(TFileReferenceTag, const TFileReferenceType&)>>(funcs);
          if (c) {
            return c(TFileReferenceTag(), value.getValue<TFileReferenceType>());
          }
          break;
        }
        case TValueType::FLOATING_POINT_ARRAY: {
          auto c = std::get<std::function<R(TFloatArrayTag, const TFloatArrayType&)>>(funcs);
          if (c) {
            return c(TFloatArrayTag(), value.getValue<TFloatArrayType>());
          }
          break;
        }
        case TValueType::BOOLEAN_ARRAY: {
          auto c = std::get<std::function<R(TBoolArrayTag, const TBoolArrayType&)>>(funcs);
          if (c) {
            return c(TBoolArrayTag(), value.getValue<TBoolArrayType>());
          }
          break;
        }
        case TValueType::INTEGER_ARRAY: {
          auto c = std::get<std::function<R(TIntArrayTag, const TIntArrayType&)>>(funcs);
          if (c) {
            return c(TIntArrayTag(), value.getValue<TIntArrayType>());
          }
          break;
        }
        case TValueType::ENUM_ARRAY: {
          auto c = std::get<std::function<R(TEnumArrayTag, const TEnumArrayType&)>>(funcs);
          if (c) {
            return c(TEnumArrayTag(), value.getValue<TEnumArrayType>());
          }
          break;
        }
        case TValueType::STRING_ARRAY: {
          auto c = std::get<std::function<R(TStringArrayTag, const TStringArrayType&)>>(funcs);
          if (c) {
            return c(TStringArrayTag(), value.getValue<TStringArrayType>());
          }
          break;
        }
        case TValueType::REFERENCE_COMPONENT: {
          auto c = std::get<std::function<R(TReferenceComponentTag, const TReferenceComponentType&)>>(funcs);
          if (c) {
            return c(TReferenceComponentTag(), value.getValue<TReferenceComponentType>());
          }
          break;
        }
        case TValueType::FLOATING_POINT_MATRIX: {
          auto c = std::get<std::function<R(TFloatMatrixTag, const TFloatMatrixType&)>>(funcs);
          if (c) {
            return c(TFloatMatrixTag(), value.getValue<TFloatMatrixType>());
          }
          break;
        }
        case TValueType::INTEGER_MATRIX: {
          auto c = std::get<std::function<R(TIntMatrixTag, const TIntMatrixType&)>>(funcs);
          if (c) {
            return c(TIntMatrixTag(), value.getValue<TIntMatrixType>());
          }
          break;
        }
        case TValueType::BOOLEAN_MATRIX: {
          auto c = std::get<std::function<R(TBoolMatrixTag, const TBoolMatrixType&)>>(funcs);
          if (c) {
            return c(TBoolMatrixTag(), value.getValue<TBoolMatrixType>());
          }
          break;
        }
        case TValueType::STRING_MATRIX: {
          auto c = std::get<std::function<R(TStringMatrixTag, const TStringMatrixType&)>>(funcs);
          if (c) {
            return c(TStringMatrixTag(), value.getValue<TStringMatrixType>());
          }
          break;
        }
        case TValueType::ARRAY_OF_INTEGER_ARRAYS: {
          auto c = std::get<std::function<R(TArrayOfIntArraysTag, const TArrayOfIntArraysType&)>>(funcs);
          if (c) {
            return c(TArrayOfIntArraysTag(), value.getValue<TArrayOfIntArraysType>());
          }
          break;
        }
      }
    } catch (const std::bad_variant_access&) {
      throw TException{fmt::format("wrong value {} for type {}", value.asString(), toTypeString(type))};
    }
    throw TException{fmt::format("no function set for {}", toTypeString(type))};
  }
}

#endif
