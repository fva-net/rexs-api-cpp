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

#ifndef REXSAPI_XML_VALUE_DECODER_HXX
#define REXSAPI_XML_VALUE_DECODER_HXX

#include <rexsapi/CodedValue.hxx>
#include <rexsapi/ConversionHelper.hxx>
#include <rexsapi/Types.hxx>
#include <rexsapi/Value.hxx>
#include <rexsapi/Xml.hxx>
#include <rexsapi/XmlUtils.hxx>
#include <rexsapi/database/EnumValues.hxx>

#include <memory>
#include <optional>
#include <unordered_map>

namespace rexsapi::detail
{
  class TXMLDecoder
  {
  public:
    virtual ~TXMLDecoder() = default;

    [[nodiscard]] std::pair<TValue, TDecoderResult> decode(const std::optional<const database::TEnumValues>& enumValue,
                                                           const pugi::xml_node& node) const
    {
      return onDecode(enumValue, node);
    }

  protected:
    TXMLDecoder() = default;

    TXMLDecoder(const TXMLDecoder&) = delete;
    TXMLDecoder& operator=(const TXMLDecoder&) = delete;
    TXMLDecoder(TXMLDecoder&&) noexcept = default;
    TXMLDecoder& operator=(TXMLDecoder&&) noexcept = delete;

  private:
    virtual std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>& enumValue,
                                                       const pugi::xml_node& node) const = 0;
  };

  class TXMLValueDecoder
  {
  public:
    TXMLValueDecoder();

    [[nodiscard]] std::pair<TValue, TDecoderResult> decode(TValueType type,
                                                           const std::optional<const database::TEnumValues>& enumValue,
                                                           const pugi::xml_node& node) const;

    [[nodiscard]] std::pair<TValue, TValueType> decodeUnknown(const pugi::xml_node& node) const;

  private:
    std::unordered_map<TValueType, std::unique_ptr<TXMLDecoder>> m_Decoder;
  };

  namespace xml
  {
    class TStringDecoder : public TXMLDecoder
    {
    public:
      using Type = std::string;

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>&,
                                                 const pugi::xml_node& node) const override
      {
        return std::make_pair(TValue{node.child_value()}, TDecoderResult::SUCCESS);
      }
    };

    class TBooleanDecoder : public TXMLDecoder
    {
    public:
      using Type = bool;

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>&,
                                                 const pugi::xml_node& node) const override
      {
        const auto* value = node.child_value();
        if (std::strncmp("true", value, 4) == 0) {
          return std::make_pair(TValue{true}, TDecoderResult::SUCCESS);
        }
        if (std::strncmp("false", value, 5) == 0) {
          return std::make_pair(TValue{false}, TDecoderResult::SUCCESS);
        }

        return std::make_pair(TValue{}, TDecoderResult::FAILURE);
      }
    };

    class TIntegerDecoder : public TXMLDecoder
    {
    public:
      using Type = int64_t;

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>&,
                                                 const pugi::xml_node& node) const override
      {
        try {
          return std::make_pair(TValue{convertToInt64(node.child_value())}, TDecoderResult::SUCCESS);
        } catch (const std::exception&) {
          return std::make_pair(TValue{}, TDecoderResult::FAILURE);
        }
      }
    };

    class TFloatDecoder : public TXMLDecoder
    {
    public:
      using Type = double;

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>&,
                                                 const pugi::xml_node& node) const override
      {
        try {
          return std::make_pair(TValue{convertToDouble(node.child_value())}, TDecoderResult::SUCCESS);
        } catch (const std::exception&) {
          return std::make_pair(TValue{}, TDecoderResult::FAILURE);
        }
      }
    };

    class TEnumDecoder : public TXMLDecoder
    {
    public:
      using Type = std::string;

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>& enumValue,
                                                 const pugi::xml_node& node) const override
      {
        if (enumValue) {
          return std::make_pair(TValue{node.child_value()}, enumValue->check(node.child_value())
                                                              ? TDecoderResult::SUCCESS
                                                              : TDecoderResult::FAILURE);
        }
        /// No enum values means this is a custom enum attribute, so accept any text
        return std::make_pair(TValue{node.child_value()}, TDecoderResult::SUCCESS);
      }
    };

    class TDatetimeDecoder : public TXMLDecoder
    {
    public:
      using Type = std::string;

    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>&,
                                                 const pugi::xml_node& node) const override
      {
        try {
          return std::make_pair(TValue{TDatetime{node.child_value()}}, TDecoderResult::SUCCESS);
        } catch (const std::exception&) {
          return std::make_pair(TValue{}, TDecoderResult::FAILURE);
        }
      }
    };

    template<typename ElementDecoder, typename ArrayType = typename ElementDecoder::Type>
    class TArrayDecoder : public TXMLDecoder
    {
    protected:
      using type = typename ElementDecoder::Type;
      using type2 = ArrayType;

      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>& enumValue,
                                                 const pugi::xml_node& node) const override
      {
        std::vector<type2> array;
        const ElementDecoder decoder;
        TDecoderResult result{TDecoderResult::SUCCESS};
        for (const auto& arrayNode : node.select_nodes("array/c")) {
          const auto [value, res] = decoder.decode(enumValue, arrayNode.node());
          if (res == TDecoderResult::SUCCESS) {
            const TValue& val = value;
            array.emplace_back(std::move(val.getValue<type>()));
          } else {
            result = TDecoderResult::FAILURE;
          }
        }
        return std::make_pair(TValue{std::move(array)}, result);
      }
    };

    template<typename ElementDecoder>
    class TCodedArrayDecoder : public TArrayDecoder<ElementDecoder>
    {
    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>& enumValue,
                                                 const pugi::xml_node& node) const override
      {
        const auto child = node.first_child();
        TValue value;
        const auto codedType = rexsapi::detail::codedValueFromString(detail::getStringAttribute(child, "code"));
        switch (codedType) {
          case detail::TCodedValueType::None:
            return TArrayDecoder<ElementDecoder>::onDecode(enumValue, node);
          case detail::TCodedValueType::Int32: {
            value = detail::TCodedValueArrayDecoder<
              typename TArrayDecoder<ElementDecoder>::type,
              detail::Enum2type<detail::to_underlying(detail::TCodedValueType::Int32)>>::decode(child.child_value());
            break;
          }
          case detail::TCodedValueType::Float32: {
            value = detail::TCodedValueArrayDecoder<
              typename TArrayDecoder<ElementDecoder>::type,
              detail::Enum2type<detail::to_underlying(detail::TCodedValueType::Float32)>>::decode(child.child_value());
            break;
          }
          case detail::TCodedValueType::Float64: {
            value = detail::TCodedValueArrayDecoder<
              typename TArrayDecoder<ElementDecoder>::type,
              detail::Enum2type<detail::to_underlying(detail::TCodedValueType::Float64)>>::decode(child.child_value());
            break;
          }
        }
        return std::make_pair(std::move(value), TDecoderResult::SUCCESS);
      }
    };

    template<typename ElementDecoder, typename MatrixType = typename ElementDecoder::Type>
    class TMatrixDecoder : public TXMLDecoder
    {
    protected:
      using type = typename ElementDecoder::Type;
      using type2 = MatrixType;

      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>& enumValue,
                                                 const pugi::xml_node& node) const override
      {
        TMatrix<type2> matrix;
        const ElementDecoder decoder;
        bool result{true};

        for (const auto& row : node.select_nodes("matrix/r")) {
          std::vector<type2> r;

          for (const auto& column : row.node().select_nodes("c")) {
            const auto [value, res] = decoder.decode(enumValue, column.node());
            if (res == TDecoderResult::SUCCESS) {
              const TValue& val = value;
              r.emplace_back(std::move(val.getValue<type>()));
            } else {
              result = false;
            }
          }

          matrix.m_Values.emplace_back(std::move(r));
        }

        result &= matrix.validate();

        return std::make_pair(TValue{std::move(matrix)}, result ? TDecoderResult::SUCCESS : TDecoderResult::FAILURE);
      }
    };

    template<typename ElementDecoder>
    class TCodedMatrixDecoder : public TMatrixDecoder<ElementDecoder>
    {
    private:
      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>& enumValue,
                                                 const pugi::xml_node& node) const override
      {
        size_t rows = 0;
        size_t columns = 0;
        const auto child = node.first_child();
        TValue value;
        const auto codedType = rexsapi::detail::codedValueFromString(detail::getStringAttribute(child, "code"));
        if (codedType != detail::TCodedValueType::None) {
          rows = convertToUint64(detail::getStringAttribute(child, "rows"));
          columns = convertToUint64(detail::getStringAttribute(child, "columns"));
        }
        switch (codedType) {
          case detail::TCodedValueType::None:
            return TMatrixDecoder<ElementDecoder>::onDecode(enumValue, node);
          case detail::TCodedValueType::Int32: {
            value = detail::TCodedValueMatrixDecoder<
              typename TMatrixDecoder<ElementDecoder>::type,
              detail::Enum2type<detail::to_underlying(detail::TCodedValueType::Int32)>>::decode(child.child_value(),
                                                                                                columns, rows);
            break;
          }
          case detail::TCodedValueType::Float32: {
            value = detail::TCodedValueMatrixDecoder<
              typename TMatrixDecoder<ElementDecoder>::type,
              detail::Enum2type<detail::to_underlying(detail::TCodedValueType::Float32)>>::decode(child.child_value(),
                                                                                                  columns, rows);
            break;
          }
          case detail::TCodedValueType::Float64: {
            value = detail::TCodedValueMatrixDecoder<
              typename TMatrixDecoder<ElementDecoder>::type,
              detail::Enum2type<detail::to_underlying(detail::TCodedValueType::Float64)>>::decode(child.child_value(),
                                                                                                  columns, rows);
            break;
          }
        }
        if (codedType != detail::TCodedValueType::None) {
          if (value.getValue<TMatrix<typename TMatrixDecoder<ElementDecoder>::type>>().m_Values.size() != rows ||
              value.getValue<TMatrix<typename TMatrixDecoder<ElementDecoder>::type>>().m_Values[0].size() != columns) {
            throw TException{"decoded matrix size does not correspond to configured size"};
          }
        }
        return std::make_pair(std::move(value), TDecoderResult::SUCCESS);
      }
    };

    template<typename ElementDecoder>
    class TArrayOfArraysDecoder : public TXMLDecoder
    {
    private:
      using type = typename ElementDecoder::Type;

      std::pair<TValue, TDecoderResult> onDecode(const std::optional<const database::TEnumValues>& enumValue,
                                                 const pugi::xml_node& node) const override
      {
        std::vector<std::vector<type>> arrays;
        const ElementDecoder decoder;
        TDecoderResult result{TDecoderResult::SUCCESS};

        for (const auto& row : node.select_nodes("array_of_arrays/array")) {
          std::vector<type> r;

          for (const auto& column : row.node().select_nodes("c")) {
            const auto [value, res] = decoder.decode(enumValue, column.node());
            if (res == TDecoderResult::SUCCESS) {
              const TValue& val = value;
              r.emplace_back(std::move(val.getValue<type>()));
            } else {
              result = TDecoderResult::FAILURE;
            }
          }

          arrays.emplace_back(std::move(r));
        }

        return std::make_pair(TValue{std::move(arrays)}, result);
      }
    };
  }


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  inline TXMLValueDecoder::TXMLValueDecoder()
  {
    m_Decoder[TValueType::BOOLEAN] = std::make_unique<xml::TBooleanDecoder>();
    m_Decoder[TValueType::INTEGER] = std::make_unique<xml::TIntegerDecoder>();
    m_Decoder[TValueType::FLOATING_POINT] = std::make_unique<xml::TFloatDecoder>();
    m_Decoder[TValueType::STRING] = std::make_unique<xml::TStringDecoder>();
    m_Decoder[TValueType::ENUM] = std::make_unique<xml::TEnumDecoder>();
    m_Decoder[TValueType::DATE_TIME] = std::make_unique<xml::TDatetimeDecoder>();
    m_Decoder[TValueType::INTEGER_ARRAY] = std::make_unique<xml::TCodedArrayDecoder<xml::TIntegerDecoder>>();
    m_Decoder[TValueType::FLOATING_POINT_ARRAY] = std::make_unique<xml::TCodedArrayDecoder<xml::TFloatDecoder>>();
    m_Decoder[TValueType::BOOLEAN_ARRAY] = std::make_unique<xml::TArrayDecoder<xml::TBooleanDecoder, Bool>>();
    m_Decoder[TValueType::ENUM_ARRAY] = std::make_unique<xml::TArrayDecoder<xml::TEnumDecoder>>();
    m_Decoder[TValueType::STRING_ARRAY] = std::make_unique<xml::TArrayDecoder<xml::TStringDecoder>>();
    m_Decoder[TValueType::FLOATING_POINT_MATRIX] = std::make_unique<xml::TCodedMatrixDecoder<xml::TFloatDecoder>>();
    m_Decoder[TValueType::INTEGER_MATRIX] = std::make_unique<xml::TCodedMatrixDecoder<xml::TIntegerDecoder>>();
    m_Decoder[TValueType::BOOLEAN_MATRIX] = std::make_unique<xml::TMatrixDecoder<xml::TBooleanDecoder, Bool>>();
    m_Decoder[TValueType::STRING_MATRIX] = std::make_unique<xml::TMatrixDecoder<xml::TStringDecoder>>();
    m_Decoder[TValueType::REFERENCE_COMPONENT] = std::make_unique<xml::TIntegerDecoder>();
    m_Decoder[TValueType::FILE_REFERENCE] = std::make_unique<xml::TStringDecoder>();
    m_Decoder[TValueType::ARRAY_OF_INTEGER_ARRAYS] =
      std::make_unique<xml::TArrayOfArraysDecoder<xml::TIntegerDecoder>>();
  }

  inline std::pair<TValue, TDecoderResult>
  TXMLValueDecoder::decode(TValueType type, const std::optional<const database::TEnumValues>& enumValue,
                           const pugi::xml_node& node) const
  {
    try {
      if (*node.child_value() == '\0' && node.first_child().empty()) {
        return std::make_pair(TValue{}, TDecoderResult::NO_VALUE);
      }
      return m_Decoder.at(type)->decode(enumValue, node);
    } catch (const std::exception&) {
      return std::make_pair(TValue{}, TDecoderResult::FAILURE);
    }
  }

  inline static bool isArray(const pugi::xml_node& node)
  {
    return !node.select_nodes("array/c").empty();
  }

  inline static bool isMatrix(const pugi::xml_node& node)
  {
    return !node.select_nodes("matrix/r").empty();
  }

  inline static bool isArrayOfArrays(const pugi::xml_node& node)
  {
    return !node.select_nodes("array_of_arrays/array").empty();
  }

  inline std::pair<TValue, TValueType> TXMLValueDecoder::decodeUnknown(const pugi::xml_node& node) const
  {
    if (isArray(node)) {
      auto [value, success] = m_Decoder.at(TValueType::STRING_ARRAY)->decode({}, node);
      return std::make_pair(std::move(value), TValueType::STRING_ARRAY);
    }
    if (isMatrix(node)) {
      auto [value, success] = m_Decoder.at(TValueType::STRING_MATRIX)->decode({}, node);
      return std::make_pair(std::move(value), TValueType::STRING_MATRIX);
    }
    if (isArrayOfArrays(node)) {
      auto [value, success] = m_Decoder.at(TValueType::ARRAY_OF_INTEGER_ARRAYS)->decode({}, node);
      return std::make_pair(std::move(value), TValueType::ARRAY_OF_INTEGER_ARRAYS);
    }

    return std::make_pair(TValue{node.child_value()}, TValueType::STRING);
  }
}

#endif
