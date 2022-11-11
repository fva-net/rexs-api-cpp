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

#ifndef REXSAPI_VALUE_DETAILS_HXX
#define REXSAPI_VALUE_DETAILS_HXX

#include <rexsapi/Types.hxx>

#include <variant>

namespace rexsapi
{
  namespace detail
  {
    template<class... Ts>
    struct overload : Ts... {
      using Ts::operator()...;
    };
    template<class... Ts>
    overload(Ts...) -> overload<Ts...>;

    using Variant =
      std::variant<std::monostate, double, bool, int64_t, std::string, std::vector<double>, std::vector<Bool>,
                   std::vector<int64_t>, std::vector<std::string>, std::vector<std::vector<int64_t>>, TMatrix<double>,
                   TMatrix<int64_t>, TMatrix<Bool>, TMatrix<std::string>>;

    template<typename T>
    inline const auto& value_getter(const Variant& value)
    {
      return std::get<T>(value);
    }

    template<>
    inline const auto& value_getter<Bool>(const Variant& value)
    {
      return std::get<bool>(value);
    }

    template<uint8_t v>
    struct Enum2type {
      enum { value = v };
    };

    template<typename T>
    constexpr typename std::underlying_type<T>::type to_underlying(T t) noexcept
    {
      return static_cast<typename std::underlying_type<T>::type>(t);
    }

    template<typename T>
    struct TypeForValueType {
      using Type = T;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::FLOATING_POINT)>> {
      using Type = double;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::BOOLEAN)>> {
      using Type = bool;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::INTEGER)>> {
      using Type = int64_t;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::ENUM)>> {
      using Type = std::string;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::STRING)>> {
      using Type = std::string;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::FILE_REFERENCE)>> {
      using Type = std::string;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::BOOLEAN_ARRAY)>> {
      using Type = std::vector<Bool>;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::FLOATING_POINT_ARRAY)>> {
      using Type = std::vector<double>;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::INTEGER_ARRAY)>> {
      using Type = std::vector<int64_t>;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::ENUM_ARRAY)>> {
      using Type = std::vector<std::string>;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::STRING_ARRAY)>> {
      using Type = std::vector<std::string>;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::REFERENCE_COMPONENT)>> {
      using Type = int64_t;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::FLOATING_POINT_MATRIX)>> {
      using Type = TMatrix<double>;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::INTEGER_MATRIX)>> {
      using Type = TMatrix<int64_t>;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::BOOLEAN_MATRIX)>> {
      using Type = TMatrix<Bool>;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::STRING_MATRIX)>> {
      using Type = TMatrix<std::string>;
    };

    template<>
    struct TypeForValueType<Enum2type<to_underlying(TValueType::ARRAY_OF_INTEGER_ARRAYS)>> {
      using Type = std::vector<std::vector<int64_t>>;
    };
  }

  using TFloatType = detail::TypeForValueType<detail::Enum2type<detail::to_underlying(TValueType::FLOATING_POINT)>>::
    Type;  //!< The C++ type for the floating_point value type. Should always be used instead of the actual C++ type.
  using TBoolType = detail::TypeForValueType<detail::Enum2type<detail::to_underlying(TValueType::BOOLEAN)>>::
    Type;  //!< The C++ type for the boolean value type. Should always be used instead of the actual C++ type.
  using TIntType = detail::TypeForValueType<detail::Enum2type<detail::to_underlying(TValueType::INTEGER)>>::
    Type;  //!< The C++ type for the integer value type. Should always be used instead of the actual C++ type.
  using TEnumType = detail::TypeForValueType<detail::Enum2type<detail::to_underlying(TValueType::ENUM)>>::
    Type;  //!< The C++ type for the enum value type. Should always be used instead of the actual C++ type.
  using TStringType = detail::TypeForValueType<detail::Enum2type<detail::to_underlying(TValueType::STRING)>>::
    Type;  //!< The C++ type for the string value type. Should always be used instead of the actual C++ type.
  using TFileReferenceType =
    detail::TypeForValueType<detail::Enum2type<detail::to_underlying(TValueType::FILE_REFERENCE)>>::
      Type;  //!< The C++ type for the file_reference value type. Should always be used instead of the actual C++ type.
  using TBoolArrayType = detail::TypeForValueType<detail::Enum2type<detail::to_underlying(TValueType::BOOLEAN_ARRAY)>>::
    Type;  //!< The C++ type for the boolean_array value type. Should always be used instead of the actual C++ type.
  using TFloatArrayType = detail::TypeForValueType<detail::Enum2type<detail::to_underlying(
    TValueType::FLOATING_POINT_ARRAY)>>::Type;  //!< The C++ type for the floating_point_array value type. Should always
                                                //!< be used instead of the actual C++ type.
  using TIntArrayType = detail::TypeForValueType<detail::Enum2type<detail::to_underlying(TValueType::INTEGER_ARRAY)>>::
    Type;  //!< The C++ type for the integer_array value type. Should always be used instead of the actual C++ type.
  using TEnumArrayType = detail::TypeForValueType<detail::Enum2type<detail::to_underlying(TValueType::ENUM_ARRAY)>>::
    Type;  //!< The C++ type for the enum_array value type. Should always be used instead of the actual C++ type.
  using TStringArrayType =
    detail::TypeForValueType<detail::Enum2type<detail::to_underlying(TValueType::STRING_ARRAY)>>::
      Type;  //!< The C++ type for the string_array value type. Should always be used instead of the actual C++ type.
  using TReferenceComponentType = detail::TypeForValueType<detail::Enum2type<detail::to_underlying(
    TValueType::REFERENCE_COMPONENT)>>::Type;  //!< The C++ type for the reference_component value type. Should always
                                               //!< be used instead of the actual C++ type.
  using TFloatMatrixType = detail::TypeForValueType<detail::Enum2type<detail::to_underlying(
    TValueType::FLOATING_POINT_MATRIX)>>::Type;  //!< The C++ type for the floating_point_matrix value type. Should
                                                 //!< always be used instead of the actual C++ type.
  using TIntMatrixType = detail::TypeForValueType<detail::Enum2type<detail::to_underlying(
    TValueType::INTEGER_MATRIX)>>::Type;  //!< The C++ type for the integer_matrix value type. Should
                                          //!< always be used instead of the actual C++ type.
  using TBoolMatrixType = detail::TypeForValueType<detail::Enum2type<detail::to_underlying(
    TValueType::BOOLEAN_MATRIX)>>::Type;  //!< The C++ type for the boolean_matrix value type. Should
                                          //!< always be used instead of the actual C++ type.
  using TStringMatrixType =
    detail::TypeForValueType<detail::Enum2type<detail::to_underlying(TValueType::STRING_MATRIX)>>::
      Type;  //!< The C++ type for the string_matrix value type. Should always be used instead of the actual C++ type.
  using TArrayOfIntArraysType = detail::TypeForValueType<detail::Enum2type<detail::to_underlying(
    TValueType::ARRAY_OF_INTEGER_ARRAYS)>>::Type;  //!< The C++ type for the array_of_integer_arrays value type. Should
                                                   //!< always be used instead of the actual C++ type.

  using TFloatTag = detail::Enum2type<detail::to_underlying(TValueType::FLOATING_POINT)>;
  using TIntTag = detail::Enum2type<detail::to_underlying(TValueType::INTEGER)>;
  using TBoolTag = detail::Enum2type<detail::to_underlying(TValueType::BOOLEAN)>;
  using TEnumTag = detail::Enum2type<detail::to_underlying(TValueType::ENUM)>;
  using TStringTag = detail::Enum2type<detail::to_underlying(TValueType::STRING)>;
  using TFileReferenceTag = detail::Enum2type<detail::to_underlying(TValueType::FILE_REFERENCE)>;
  using TFloatArrayTag = detail::Enum2type<detail::to_underlying(TValueType::FLOATING_POINT_ARRAY)>;
  using TBoolArrayTag = detail::Enum2type<detail::to_underlying(TValueType::BOOLEAN_ARRAY)>;
  using TIntArrayTag = detail::Enum2type<detail::to_underlying(TValueType::INTEGER_ARRAY)>;
  using TEnumArrayTag = detail::Enum2type<detail::to_underlying(TValueType::ENUM_ARRAY)>;
  using TStringArrayTag = detail::Enum2type<detail::to_underlying(TValueType::STRING_ARRAY)>;
  using TReferenceComponentTag = detail::Enum2type<detail::to_underlying(TValueType::REFERENCE_COMPONENT)>;
  using TFloatMatrixTag = detail::Enum2type<detail::to_underlying(TValueType::FLOATING_POINT_MATRIX)>;
  using TIntMatrixTag = detail::Enum2type<detail::to_underlying(TValueType::INTEGER_MATRIX)>;
  using TBoolMatrixTag = detail::Enum2type<detail::to_underlying(TValueType::BOOLEAN_MATRIX)>;
  using TStringMatrixTag = detail::Enum2type<detail::to_underlying(TValueType::STRING_MATRIX)>;
  using TArrayOfIntArraysTag = detail::Enum2type<detail::to_underlying(TValueType::ARRAY_OF_INTEGER_ARRAYS)>;
}

#endif
