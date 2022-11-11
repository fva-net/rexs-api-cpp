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

#ifndef REXSAPI_JSON_VALUE_DECODER_HXX
#define REXSAPI_JSON_VALUE_DECODER_HXX

#include <rexsapi/CodedValue.hxx>
#include <rexsapi/ConversionHelper.hxx>
#include <rexsapi/Json.hxx>
#include <rexsapi/Types.hxx>
#include <rexsapi/Value.hxx>
#include <rexsapi/database/EnumValues.hxx>

#include <memory>
#include <optional>
#include <unordered_map>

namespace rexsapi::detail
{
  namespace json
  {
    class TJsonDecoder;
  }

  class TJsonValueDecoder
  {
  public:
    TJsonValueDecoder();

    [[nodiscard]] std::pair<TValue, TDecoderResult> decode(TValueType type,
                                                           const std::optional<const database::TEnumValues>& enumValue,
                                                           const rexsapi::json& node) const;

  private:
    std::unordered_map<TValueType, std::unique_ptr<json::TJsonDecoder>> m_Decoder;
  };

  namespace json
  {
    static inline bool is_coded(const rexsapi::json& node) noexcept;
    static inline TValueType getType(const rexsapi::json& node);

    class TJsonDecoder
    {
    public:
      virtual ~TJsonDecoder() = default;

      [[nodiscard]] std::pair<TValue, TDecoderResult>
      decode(const std::optional<const database::TEnumValues>& enumValue, const rexsapi::json& node) const
      {
        return onDecode(enumValue, node);
      }

      virtual bool isEmpty(const rexsapi::json& node) const noexcept = 0;

    protected:
      TJsonDecoder() = default;

      TJsonDecoder(const TJsonDecoder&) = delete;
      TJsonDecoder& operator=(const TJsonDecoder&) = delete;
      TJsonDecoder(TJsonDecoder&&) noexcept = default;
      TJsonDecoder& operator=(TJsonDecoder&&) noexcept = delete;

    private:
      virtual std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>& enumValue,
                                                         const rexsapi::json& node) const = 0;
    };

    class TStringDecoder : public TJsonDecoder
    {
    public:
      using Type = std::string;

      bool isEmpty(const rexsapi::json& node) const noexcept override
      {
        return node.at("string").is_null();
      }

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>&,
                                                 const rexsapi::json& node) const override
      {
        auto value = node.at("string").get<std::string>();
        return std::make_pair(TValue{value}, !value.empty() ? TDecoderResult::SUCCESS : TDecoderResult::FAILURE);
      }
    };

    class TFileReferenceDecoder : public TJsonDecoder
    {
    public:
      using Type = std::string;

      bool isEmpty(const rexsapi::json& node) const noexcept override
      {
        return node.at("file_reference").is_null();
      }

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>&,
                                                 const rexsapi::json& node) const override
      {
        auto value = node.at("file_reference").get<std::string>();
        return std::make_pair(TValue{value}, !value.empty() ? TDecoderResult::SUCCESS : TDecoderResult::FAILURE);
      }
    };

    class TBooleanDecoder : public TJsonDecoder
    {
    public:
      using Type = bool;

      bool isEmpty(const rexsapi::json& node) const noexcept override
      {
        return node.at("boolean").is_null();
      }

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>&,
                                                 const rexsapi::json& node) const override
      {
        return std::make_pair(TValue{node.at("boolean").get<bool>()}, TDecoderResult::SUCCESS);
      }
    };

    class TIntegerDecoder : public TJsonDecoder
    {
    public:
      using Type = int64_t;

      bool isEmpty(const rexsapi::json& node) const noexcept override
      {
        return node.at("integer").is_null();
      }

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>&,
                                                 const rexsapi::json& node) const override
      {
        return std::make_pair(TValue{node.at("integer").get<int64_t>()}, TDecoderResult::SUCCESS);
      }
    };

    class TReferenceDecoder : public TJsonDecoder
    {
    public:
      using Type = int64_t;

      bool isEmpty(const rexsapi::json& node) const noexcept override
      {
        return node.at("reference_component").is_null();
      }

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>&,
                                                 const rexsapi::json& node) const override
      {
        return std::make_pair(TValue{node.at("reference_component").get<int64_t>()}, TDecoderResult::SUCCESS);
      }
    };

    class TFloatDecoder : public TJsonDecoder
    {
    public:
      using Type = double;

      bool isEmpty(const rexsapi::json& node) const noexcept override
      {
        return node.at("floating_point").is_null();
      }

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>&,
                                                 const rexsapi::json& node) const override
      {
        return std::make_pair(TValue{node.at("floating_point").get<double>()}, TDecoderResult::SUCCESS);
      }
    };

    class TEnumDecoder : public TJsonDecoder
    {
    public:
      using Type = std::string;

      bool isEmpty(const rexsapi::json& node) const noexcept override
      {
        return node.at("enum").is_null();
      }

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>& enumValue,
                                                 const rexsapi::json& node) const override
      {
        auto value = node.at("enum").get<std::string>();

        if (enumValue) {
          return std::make_pair(TValue{value},
                                enumValue->check(value) ? TDecoderResult::SUCCESS : TDecoderResult::FAILURE);
        }
        /// No enum values means this is a custom enum attribute, so accept any text
        return std::make_pair(TValue{value}, TDecoderResult::SUCCESS);
      }
    };

    template<typename Type, typename ArrayType = Type>
    class TArrayDecoder : public TJsonDecoder
    {
    public:
      using type = Type;
      using type2 = ArrayType;

      explicit TArrayDecoder(std::string name)
      : m_Name{std::move(name)}
      {
      }

      bool isEmpty(const rexsapi::json& node) const noexcept override
      {
        return node.at(m_Name).is_null();
      }

    protected:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>&,
                                                 const rexsapi::json& node) const override
      {
        std::vector<ArrayType> array;

        const auto& arr = node.at(m_Name);
        for (const auto& element : arr) {
          array.emplace_back(element.template get<type>());
        }
        return std::make_pair(TValue{array}, TDecoderResult::SUCCESS);
      }

      std::string m_Name;
    };

    template<typename Type>
    class TCodedArrayDecoder : public TArrayDecoder<Type>
    {
    public:
      explicit TCodedArrayDecoder(std::string name)
      : TArrayDecoder<Type>{std::move(name)}
      {
      }

      bool isEmpty(const rexsapi::json& node) const noexcept override
      {
        if (is_coded(node)) {
          return node.at(TArrayDecoder<Type>::m_Name + "_coded").is_null();
        }
        return node.at(TArrayDecoder<Type>::m_Name).is_null();
      }

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>& enumValue,
                                                 const rexsapi::json& node) const override
      {
        if (is_coded(node)) {
          TValue value;
          const auto& coded = node.at(TArrayDecoder<Type>::m_Name + "_coded");
          const auto codedType = detail::codedValueFromString(coded["code"].template get<std::string>());
          const auto& val = coded["value"].template get<std::string>();
          switch (codedType) {
            case detail::TCodedValueType::None:
              break;
            case detail::TCodedValueType::Int32: {
              value =
                detail::TCodedValueArrayDecoder<Type, Enum2type<to_underlying(detail::TCodedValueType::Int32)>>::decode(
                  val);
              break;
            }
            case detail::TCodedValueType::Float32: {
              value = detail::TCodedValueArrayDecoder<
                Type, Enum2type<to_underlying(detail::TCodedValueType::Float32)>>::decode(val);
              break;
            }
            case detail::TCodedValueType::Float64: {
              value = detail::TCodedValueArrayDecoder<
                Type, Enum2type<to_underlying(detail::TCodedValueType::Float64)>>::decode(val);
              break;
            }
          }
          return std::make_pair(std::move(value), TDecoderResult::SUCCESS);
        }

        return TArrayDecoder<Type>::onDecode(enumValue, node);
      }
    };

    class TEnumArrayDecoder : public TJsonDecoder
    {
    public:
      bool isEmpty(const rexsapi::json& node) const noexcept override
      {
        return node.at("enum_array").is_null();
      }

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>& enumValue,
                                                 const rexsapi::json& node) const override
      {
        if (enumValue.has_value()) {
          std::vector<std::string> array;
          bool result{true};
          const auto arr = node.at("enum_array");
          for (const auto& element : arr) {
            auto value = element.get<std::string>();
            if (enumValue->check(value)) {
              array.emplace_back(value);
            } else {
              result = false;
            }
          }
          return std::make_pair(TValue{std::move(array)}, result ? TDecoderResult::SUCCESS : TDecoderResult::FAILURE);
        }
        return std::make_pair(TValue{}, TDecoderResult::FAILURE);
      }
    };

    template<typename Type, typename MatrixType = Type>
    class TMatrixDecoder : public TJsonDecoder
    {
    public:
      using type = Type;
      using type2 = MatrixType;

      explicit TMatrixDecoder(std::string name)
      : m_Name{std::move(name)}
      {
      }

      bool isEmpty(const rexsapi::json& node) const noexcept override
      {
        return node.at(m_Name).is_null();
      }

    protected:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>&,
                                                 const rexsapi::json& node) const override
      {
        TMatrix<type2> matrix;
        for (const auto& row : node.at(m_Name)) {
          std::vector<type2> r;
          for (const auto& column : row) {
            r.emplace_back(column.template get<type>());
          }
          matrix.m_Values.emplace_back(std::move(r));
        }
        bool result = matrix.validate();
        return std::make_pair(TValue{std::move(matrix)}, result ? TDecoderResult::SUCCESS : TDecoderResult::FAILURE);
      }
      std::string m_Name;
    };

    template<typename Type>
    class TCodedMatrixDecoder : public TMatrixDecoder<Type>
    {
    public:
      using type = Type;

      explicit TCodedMatrixDecoder(std::string name)
      : TMatrixDecoder<Type>{std::move(name)}
      {
      }

      bool isEmpty(const rexsapi::json& node) const noexcept override
      {
        if (is_coded(node)) {
          return node.at(TMatrixDecoder<Type>::m_Name + "_coded").is_null();
        }
        return node.at(TMatrixDecoder<Type>::m_Name).is_null();
      }

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>& enumValue,
                                                 const rexsapi::json& node) const override
      {
        if (is_coded(node)) {
          TValue value;
          const auto& coded = node.at(TMatrixDecoder<Type>::m_Name + "_coded");
          const auto codedType = detail::codedValueFromString(coded["code"].template get<std::string>());
          const auto rows = coded["rows"].template get<uint64_t>();
          const auto columns = coded["columns"].template get<uint64_t>();
          if (rows != columns) {
            throw TException{"matrix rows != columns"};
          }
          const auto& val = coded["value"].template get<std::string>();
          switch (codedType) {
            case detail::TCodedValueType::None:
              throw TException{"unknown code"};
            case detail::TCodedValueType::Int32: {
              value =
                detail::TCodedValueMatrixDecoder<Type,
                                                 Enum2type<to_underlying(detail::TCodedValueType::Int32)>>::decode(val);
              break;
            }
            case detail::TCodedValueType::Float32: {
              value = detail::TCodedValueMatrixDecoder<
                Type, Enum2type<to_underlying(detail::TCodedValueType::Float32)>>::decode(val);
              break;
            }
            case detail::TCodedValueType::Float64: {
              value = detail::TCodedValueMatrixDecoder<
                Type, Enum2type<to_underlying(detail::TCodedValueType::Float64)>>::decode(val);
              break;
            }
          }
          if (value.getValue<TMatrix<Type>>().m_Values.size() != rows) {
            throw TException{"decoded matrix size does not correspond to configured size"};
          }
          return std::make_pair(std::move(value), TDecoderResult::SUCCESS);
        }

        return TMatrixDecoder<Type>::onDecode(enumValue, node);
      }
    };

    template<typename Type>
    class TArrayOfArraysDecoder : public TJsonDecoder
    {
    public:
      using type = Type;

      explicit TArrayOfArraysDecoder(std::string name)
      : m_Name{std::move(name)}
      {
      }

      bool isEmpty(const rexsapi::json& node) const noexcept override
      {
        return node.at(m_Name).is_null();
      }

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>&,
                                                 const rexsapi::json& node) const override
      {
        std::vector<std::vector<type>> arrays;
        for (const auto& row : node.at(m_Name)) {
          std::vector<type> r;
          for (const auto& column : row) {
            r.emplace_back(column.template get<type>());
          }
          arrays.emplace_back(std::move(r));
        }

        return std::make_pair(TValue{std::move(arrays)}, TDecoderResult::SUCCESS);
      }
      std::string m_Name;
    };
  }


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  inline TJsonValueDecoder::TJsonValueDecoder()
  {
    m_Decoder[TValueType::BOOLEAN] = std::make_unique<json::TBooleanDecoder>();
    m_Decoder[TValueType::INTEGER] = std::make_unique<json::TIntegerDecoder>();
    m_Decoder[TValueType::FLOATING_POINT] = std::make_unique<json::TFloatDecoder>();
    m_Decoder[TValueType::STRING] = std::make_unique<json::TStringDecoder>();
    m_Decoder[TValueType::ENUM] = std::make_unique<json::TEnumDecoder>();
    m_Decoder[TValueType::INTEGER_ARRAY] = std::make_unique<json::TCodedArrayDecoder<int64_t>>("integer_array");
    m_Decoder[TValueType::FLOATING_POINT_ARRAY] =
      std::make_unique<json::TCodedArrayDecoder<double>>("floating_point_array");
    m_Decoder[TValueType::BOOLEAN_ARRAY] = std::make_unique<json::TArrayDecoder<bool, Bool>>("boolean_array");
    m_Decoder[TValueType::STRING_ARRAY] = std::make_unique<json::TArrayDecoder<std::string>>("string_array");
    m_Decoder[TValueType::ENUM_ARRAY] = std::make_unique<json::TEnumArrayDecoder>();
    m_Decoder[TValueType::FLOATING_POINT_MATRIX] =
      std::make_unique<json::TCodedMatrixDecoder<double>>("floating_point_matrix");
    m_Decoder[TValueType::INTEGER_MATRIX] = std::make_unique<json::TCodedMatrixDecoder<int64_t>>("integer_matrix");
    m_Decoder[TValueType::BOOLEAN_MATRIX] = std::make_unique<json::TMatrixDecoder<bool, Bool>>("boolean_matrix");
    m_Decoder[TValueType::STRING_MATRIX] = std::make_unique<json::TMatrixDecoder<std::string>>("string_matrix");
    m_Decoder[TValueType::REFERENCE_COMPONENT] = std::make_unique<json::TReferenceDecoder>();
    m_Decoder[TValueType::FILE_REFERENCE] = std::make_unique<json::TFileReferenceDecoder>();
    m_Decoder[TValueType::ARRAY_OF_INTEGER_ARRAYS] =
      std::make_unique<json::TArrayOfArraysDecoder<int64_t>>("array_of_integer_arrays");
  }

  static inline TValueType json::getType(const rexsapi::json& node)
  {
    for (const auto& [key, _] : node.items()) {
      if (key == "id" || key == "unit") {
        continue;
      }
      if (key == "floating_point_array_coded") {
        return TValueType::FLOATING_POINT_ARRAY;
      }
      if (key == "integer_array_coded") {
        return TValueType::INTEGER_ARRAY;
      }
      if (key == "floating_point_matrix_coded") {
        return TValueType::FLOATING_POINT_MATRIX;
      }
      return typeFromString(key);
    }
    throw TException{fmt::format("no type found in node")};
  }

  static inline bool json::is_coded(const rexsapi::json& node) noexcept
  {
    for (const auto& [key, _] : node.items()) {
      if (key == "id" || key == "unit") {
        continue;
      }
      if (key == "floating_point_array_coded" || key == "integer_array_coded" || key == "floating_point_matrix_coded") {
        return true;
      }
    }
    return false;
  }

  inline std::pair<TValue, TDecoderResult>
  TJsonValueDecoder::decode(TValueType type, const std::optional<const database::TEnumValues>& enumValue,
                            const rexsapi::json& node) const
  {
    try {
      if (json::getType(node) != type) {
        return std::pair(TValue{}, TDecoderResult::WRONG_TYPE);
      }

      if (m_Decoder.at(type)->isEmpty(node)) {
        return std::make_pair(TValue{}, TDecoderResult::NO_VALUE);
      }
      return m_Decoder.at(type)->decode(enumValue, node);
    } catch (const std::exception&) {
      return std::make_pair(TValue{}, TDecoderResult::FAILURE);
    }
  }
}

#endif
