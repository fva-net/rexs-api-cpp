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

#ifndef REXSAPI_CODED_VALUE_HXX
#define REXSAPI_CODED_VALUE_HXX

#include <rexsapi/Base64.hxx>
#include <rexsapi/Value.hxx>


/**
 * @brief Classes and functions in the detail namespace shall not be used by client code
 *
 */
namespace rexsapi::detail
{
  enum class TCodedValueType : uint8_t { None, Int32, Float32, Float64 };

  static TCodedValueType codedValueFromString(const std::string& type);
  static std::string toCodedValueString(TCodedValueType type);


  template<typename T, std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>* = nullptr>
  class TCodedValueArray
  {
  public:
    static std::string encode(const std::vector<T>& array)
    {
      const auto* data = reinterpret_cast<const uint8_t*>(array.data());
      const auto len = array.size() * sizeof(T);
      return base64Encode(data, len);
    }

    static std::vector<T> decode(std::string_view value)
    {
      std::vector<T> array;
      const auto data = base64Decode(value);
      const auto count = data.size() / sizeof(T);
      array.reserve(count);
      const auto values = reinterpret_cast<const T*>(data.data());
      for (size_t n = 0; n < count; ++n) {
        array.emplace_back(values[n]);
      }

      return array;
    }
  };

  template<typename T, std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>* = nullptr>
  class TCodedValueMatrix
  {
  public:
    static std::string encode(const TMatrix<T>& matrix)
    {
      const auto count = matrix.m_Values.size() * matrix.m_Values.size() * sizeof(T);
      std::vector<T> array;
      array.reserve(count);
      for (const auto& row : matrix.m_Values) {
        for (const auto& column : row) {
          array.emplace_back(column);
        }
      }
      const auto* data = reinterpret_cast<const uint8_t*>(array.data());
      const auto len = array.size() * sizeof(T);
      return base64Encode(data, len);
    }

    static TMatrix<T> decode(std::string_view value)
    {
      TMatrix<T> matrix;
      const auto data = base64Decode(value);
      const auto count = data.size() / sizeof(T);
      const auto elementCount = static_cast<size_t>(::sqrt(static_cast<double>(count)));
      const auto values = reinterpret_cast<const T*>(data.data());
      matrix.m_Values.reserve(elementCount);
      for (size_t row = 0; row < elementCount; ++row) {
        std::vector<T> col;
        col.reserve(elementCount);
        for (size_t column = 0; column < elementCount; ++column) {
          col.emplace_back(values[(row * elementCount) + column]);
        }
        matrix.m_Values.emplace_back(std::move(col));
      }

      return matrix;
    }
  };


  template<typename T>
  inline std::pair<std::string, TCodedValueType> encodeArray(const std::vector<T>&, TCodeType);

  template<>
  inline std::pair<std::string, TCodedValueType> encodeArray(const std::vector<int64_t>& array, TCodeType)
  {
    std::vector<int32_t> tmp;
    tmp.resize(array.size());
    std::transform(array.begin(), array.end(), tmp.begin(), [](auto x) {
      return static_cast<int32_t>(x);
    });
    return std::make_pair(TCodedValueArray<int32_t>::encode(tmp), TCodedValueType::Int32);
  }

  template<>
  inline std::pair<std::string, TCodedValueType> encodeArray(const std::vector<double>& array, TCodeType type)
  {
    if (type == TCodeType::Default) {
      return std::make_pair(TCodedValueArray<double>::encode(array), TCodedValueType::Float64);
    }
    if (type == TCodeType::Optimized) {
      std::vector<float> tmp;
      tmp.resize(array.size());
      std::transform(array.begin(), array.end(), tmp.begin(), [](auto x) {
        return static_cast<float>(x);
      });
      return std::make_pair(TCodedValueArray<float>::encode(tmp), TCodedValueType::Float32);
    }
    throw TException{"should never reach this"};
  }

  template<typename T>
  inline std::pair<std::string, TCodedValueType> encodeMatrix(const TMatrix<T>&, TCodeType);

  template<>
  inline std::pair<std::string, TCodedValueType> encodeMatrix(const TMatrix<double>& matrix, TCodeType type)
  {
    if (type == TCodeType::Default) {
      return std::make_pair(TCodedValueMatrix<double>::encode(matrix), TCodedValueType::Float64);
    }
    if (type == TCodeType::Optimized) {
      TMatrix<float> tmp{matrix};
      return std::make_pair(TCodedValueMatrix<float>::encode(tmp), TCodedValueType::Float32);
    }
    throw TException{"should never reach this"};
  }

  template<>
  inline std::pair<std::string, TCodedValueType> encodeMatrix(const TMatrix<int64_t>& matrix, TCodeType)
  {
    TMatrix<int32_t> tmp{matrix};
    return std::make_pair(TCodedValueMatrix<int32_t>::encode(tmp), TCodedValueType::Int32);
  }


  template<typename T>
  struct TypeForCodedValueType {
  };

  template<>
  struct TypeForCodedValueType<Enum2type<to_underlying(TCodedValueType::Int32)>> {
    using Type = int32_t;
  };

  template<>
  struct TypeForCodedValueType<Enum2type<to_underlying(TCodedValueType::Float32)>> {
    using Type = float;
  };

  template<>
  struct TypeForCodedValueType<Enum2type<to_underlying(TCodedValueType::Float64)>> {
    using Type = double;
  };

  template<typename T>
  struct ValueTypeForCodedValueType {
  };

  template<>
  struct ValueTypeForCodedValueType<Enum2type<to_underlying(TCodedValueType::Int32)>> {
    using Type = int64_t;
  };

  template<>
  struct ValueTypeForCodedValueType<Enum2type<to_underlying(TCodedValueType::Float32)>> {
    using Type = double;
  };

  template<>
  struct ValueTypeForCodedValueType<Enum2type<to_underlying(TCodedValueType::Float64)>> {
    using Type = double;
  };

  static inline TCodeType getCodedType(TCodedValueType type)
  {
    if (type == TCodedValueType::Float32) {
      return TCodeType::Optimized;
    }
    return TCodeType::Default;
  }

  template<typename T1, typename T2>
  struct TCodedValueArrayDecoder {
    static TValue decode(std::string_view value)
    {
      if (!std::is_same_v<T1, typename ValueTypeForCodedValueType<T2>::Type>) {
        throw TException{"coded value type does not correspond to attribute value type"};
      }
      auto result = TCodedValueArray<typename TypeForCodedValueType<T2>::Type>::decode(value);
      std::vector<T1> tmp;
      tmp.resize(result.size());
      std::transform(result.begin(), result.end(), tmp.begin(), [](auto x) {
        return static_cast<T1>(x);
      });
      TValue val{std::move(tmp)};
      val.coded(getCodedType(TCodedValueType{T2::value}));
      return val;
    }
  };

  template<typename T1, typename T2>
  struct TCodedValueMatrixDecoder {
    static TValue decode(std::string_view value)
    {
      if (!std::is_same_v<T1, typename ValueTypeForCodedValueType<T2>::Type>) {
        throw TException{"coded value type does not correspond to attribute value type"};
      }
      auto result = TCodedValueMatrix<typename TypeForCodedValueType<T2>::Type>::decode(value);
      TValue val{TMatrix<T1>{result}};
      val.coded(getCodedType(TCodedValueType{T2::value}));
      return val;
    }
  };


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  static inline TCodedValueType codedValueFromString(const std::string& type)
  {
    if (type.empty()) {
      return TCodedValueType::None;
    }
    if (type == "int32") {
      return TCodedValueType::Int32;
    }
    if (type == "float32") {
      return TCodedValueType::Float32;
    }
    if (type == "float64") {
      return TCodedValueType::Float64;
    }
    throw TException{fmt::format("unknown value type '{}'", type)};
  }

  static inline std::string toCodedValueString(TCodedValueType type)
  {
    switch (type) {
      case TCodedValueType::None:
        return "none";
      case TCodedValueType::Int32:
        return "int32";
      case TCodedValueType::Float32:
        return "float32";
      case TCodedValueType::Float64:
        return "float64";
    }
    throw TException{fmt::format("unknown value type '{}'", static_cast<int64_t>(type))};
  }
}

#endif
