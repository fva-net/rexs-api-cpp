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

#include <rexsapi/JsonValueDecoder.hxx>

#include <vector>

#include <doctest.h>

namespace
{
  auto getNode(const rexsapi::json& doc, const std::string& type)
  {
    return doc[rexsapi::json::json_pointer("/" + type)];
  }
}

TEST_CASE("Json value decoder test")
{
  rexsapi::detail::TJsonValueDecoder decoder;
  std::optional<rexsapi::database::TEnumValues> enumValue;

  std::string document = R"(
  {
    "boolean": { "boolean": true },
    "float": { "floating_point": 47.11 },
    "integer": { "integer": 42 },
    "string": { "string": "This is a string" },
    "enum": { "enum": "injection_lubrication" },
    "reference component": { "reference_component": 17 },
    "file reference": { "file_reference": "/root/my/path" },
    "date time": { "date_time": "2022-06-05T04:50:27+04:00" },
    "float array": { "floating_point_array": [1.0, 2.0, 3.0] },
    "coded float array": { "floating_point_array_coded": { "code": "float32", "value": "MveeQZ6hM0I" } },
    "coded float64 array": { "floating_point_array_coded": { "code": "float64", "value": "AAAAAAAAHEAAAAAAAAAgQAAAAAAAACJA" } },
    "integer array": { "integer_array": [1, 2, 3] },
    "coded integer array": { "integer_array_coded": { "code": "int32", "value": "AQAAAAIAAAADAAAABAAAAAUAAAAGAAAABwAAAAgAAAA=" } },
    "boolean array": { "boolean_array": [true, false, false] },
    "string array": { "string_array": ["a", "b", "c"] },
    "enum array": { "enum_array": ["injection_lubrication", "dip_lubrication", "dip_lubrication"] },
    "float matrix": { "floating_point_matrix": [[1.1, 1.2, 1.3], [2.1, 2.2, 2.3], [3.1, 3.2, 3.3]] },
    "coded float matrix": { "floating_point_matrix_coded": { "code": "float64", "rows": 3, "columns": 3, "value": "AAAAAAAA8D8AAAAAAAAAQAAAAAAAAAhAAAAAAAAAEEAAAAAAAAAUQAAAAAAAABhAAAAAAAAAHEAAAAAAAAAgQAAAAAAAACJA" } },
    "coded float32 matrix": { "floating_point_matrix_coded": { "code": "float32", "rows": 3, "columns": 3, "value": "AACAPwAAAEAAAEBAAACAQAAAoEAAAMBAAADgQAAAAEEAABBB" } },
    "string matrix": { "string_matrix": [["a", "b"], ["c", "d"], ["e", "f"]] },
    "array of integer arrays": { "array_of_integer_arrays": [[1, 1, 1], [2, 2], [3]] }
  }
  )";
  std::vector<uint8_t> buffer{document.begin(), document.end()};
  rexsapi::json doc = rexsapi::json::parse(buffer);

  SUBCASE("Is coded")
  {
    CHECK_FALSE(rexsapi::detail::json::is_coded(getNode(doc, "boolean")));
    CHECK_FALSE(rexsapi::detail::json::is_coded(getNode(doc, "float matrix")));
    CHECK(rexsapi::detail::json::is_coded(getNode(doc, "coded float matrix")));
    CHECK(rexsapi::detail::json::is_coded(getNode(doc, "coded float64 array")));
    CHECK(rexsapi::detail::json::is_coded(getNode(doc, "coded integer array")));
  }

  SUBCASE("Get type")
  {
    CHECK(rexsapi::detail::json::getType(getNode(doc, "boolean")) == rexsapi::TValueType::BOOLEAN);
    CHECK(rexsapi::detail::json::getType(getNode(doc, "integer")) == rexsapi::TValueType::INTEGER);
    CHECK(rexsapi::detail::json::getType(getNode(doc, "string")) == rexsapi::TValueType::STRING);
    CHECK(rexsapi::detail::json::getType(getNode(doc, "enum")) == rexsapi::TValueType::ENUM);
    CHECK(rexsapi::detail::json::getType(getNode(doc, "reference component")) ==
          rexsapi::TValueType::REFERENCE_COMPONENT);
    CHECK(rexsapi::detail::json::getType(getNode(doc, "file reference")) == rexsapi::TValueType::FILE_REFERENCE);
    CHECK(rexsapi::detail::json::getType(getNode(doc, "date time")) == rexsapi::TValueType::DATE_TIME);
    CHECK(rexsapi::detail::json::getType(getNode(doc, "float array")) == rexsapi::TValueType::FLOATING_POINT_ARRAY);
    CHECK(rexsapi::detail::json::getType(getNode(doc, "coded float array")) ==
          rexsapi::TValueType::FLOATING_POINT_ARRAY);
    CHECK(rexsapi::detail::json::getType(getNode(doc, "integer array")) == rexsapi::TValueType::INTEGER_ARRAY);
    CHECK(rexsapi::detail::json::getType(getNode(doc, "float matrix")) == rexsapi::TValueType::FLOATING_POINT_MATRIX);
    CHECK(rexsapi::detail::json::getType(getNode(doc, "coded float matrix")) ==
          rexsapi::TValueType::FLOATING_POINT_MATRIX);
    CHECK(rexsapi::detail::json::getType(getNode(doc, "array of integer arrays")) ==
          rexsapi::TValueType::ARRAY_OF_INTEGER_ARRAYS);

    rexsapi::json node{};
    CHECK_THROWS(rexsapi::detail::json::getType(node));
  }

  SUBCASE("Decode boolean")
  {
    const auto [value, result] = decoder.decode(rexsapi::TValueType::BOOLEAN, enumValue, getNode(doc, "boolean"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<rexsapi::Bool>());
  }

  SUBCASE("Decode integer")
  {
    const auto [value, result] = decoder.decode(rexsapi::TValueType::INTEGER, enumValue, getNode(doc, "integer"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<int64_t>() == 42);
  }

  SUBCASE("Decode float")
  {
    const auto [value, result] = decoder.decode(rexsapi::TValueType::FLOATING_POINT, enumValue, getNode(doc, "float"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<double>() == doctest::Approx(47.11));
  }

  SUBCASE("Decode string")
  {
    const auto [value, result] = decoder.decode(rexsapi::TValueType::STRING, enumValue, getNode(doc, "string"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<std::string>() == "This is a string");
  }

  SUBCASE("Decode reference component")
  {
    const auto [value, result] =
      decoder.decode(rexsapi::TValueType::REFERENCE_COMPONENT, enumValue, getNode(doc, "reference component"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<int64_t>() == 17);
  }

  SUBCASE("Decode file reference")
  {
    const auto [value, result] =
      decoder.decode(rexsapi::TValueType::FILE_REFERENCE, enumValue, getNode(doc, "file reference"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<std::string>() == "/root/my/path");
  }

  SUBCASE("Decode enumValue")
  {
    enumValue = rexsapi::database::TEnumValues{{rexsapi::database::TEnumValue{"dip_lubrication", ""},
                                                rexsapi::database::TEnumValue{"injection_lubrication", ""}}};

    const auto [value, result] = decoder.decode(rexsapi::TValueType::ENUM, enumValue, getNode(doc, "enum"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<std::string>() == "injection_lubrication");
  }

  SUBCASE("Decode date time")
  {
    const auto [value, result] = decoder.decode(rexsapi::TValueType::DATE_TIME, enumValue, getNode(doc, "date time"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<rexsapi::TDatetime>().asUTCString() == "2022-06-05T00:50:27+00:00");
  }

  SUBCASE("Decode integer array")
  {
    const auto [value, result] =
      decoder.decode(rexsapi::TValueType::INTEGER_ARRAY, enumValue, getNode(doc, "integer array"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    REQUIRE(value.getValue<std::vector<int64_t>>().size() == 3);
    CHECK(value.getValue<std::vector<int64_t>>()[1] == 2);
  }

  SUBCASE("Decode coded integer array")
  {
    const auto [value, result] =
      decoder.decode(rexsapi::TValueType::INTEGER_ARRAY, enumValue, getNode(doc, "coded integer array"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    REQUIRE(value.getValue<std::vector<int64_t>>().size() == 8);
    CHECK(value.getValue<std::vector<int64_t>>()[1] == 2);
  }

  SUBCASE("Decode float array")
  {
    const auto [value, result] =
      decoder.decode(rexsapi::TValueType::FLOATING_POINT_ARRAY, enumValue, getNode(doc, "float array"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<std::vector<double>>().size() == 3);
  }

  SUBCASE("Decode coded float32 array")
  {
    const auto [value, result] =
      decoder.decode(rexsapi::TValueType::FLOATING_POINT_ARRAY, enumValue, getNode(doc, "coded float array"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<std::vector<double>>().size() == 2);
  }

  SUBCASE("Decode coded float64 array")
  {
    const auto [value, result] =
      decoder.decode(rexsapi::TValueType::FLOATING_POINT_ARRAY, enumValue, getNode(doc, "coded float64 array"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<std::vector<double>>().size() == 3);
  }

  SUBCASE("Decode boolean array")
  {
    const auto [value, result] =
      decoder.decode(rexsapi::TValueType::BOOLEAN_ARRAY, enumValue, getNode(doc, "boolean array"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<std::vector<rexsapi::Bool>>().size() == 3);
  }

  SUBCASE("Decode string array")
  {
    const auto [value, result] =
      decoder.decode(rexsapi::TValueType::STRING_ARRAY, enumValue, getNode(doc, "string array"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<std::vector<std::string>>().size() == 3);
  }

  SUBCASE("Decode enumValue array")
  {
    enumValue = rexsapi::database::TEnumValues{{rexsapi::database::TEnumValue{"dip_lubrication", ""},
                                                rexsapi::database::TEnumValue{"injection_lubrication", ""}}};

    const auto [value, result] = decoder.decode(rexsapi::TValueType::ENUM_ARRAY, enumValue, getNode(doc, "enum array"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<std::vector<std::string>>().size() == 3);
  }

  SUBCASE("Decode float matrix")
  {
    const auto [value, result] =
      decoder.decode(rexsapi::TValueType::FLOATING_POINT_MATRIX, enumValue, getNode(doc, "float matrix"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<rexsapi::TMatrix<double>>().m_Values.size() == 3);
    for (const auto& row : value.getValue<rexsapi::TMatrix<double>>().m_Values) {
      CHECK(row.size() == 3);
    }
  }

  SUBCASE("Decode coded float64 matrix")
  {
    const auto [value, result] =
      decoder.decode(rexsapi::TValueType::FLOATING_POINT_MATRIX, enumValue, getNode(doc, "coded float matrix"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<rexsapi::TMatrix<double>>().m_Values.size() == 3);
    for (const auto& row : value.getValue<rexsapi::TMatrix<double>>().m_Values) {
      CHECK(row.size() == 3);
    }
  }

  SUBCASE("Decode coded float32 matrix")
  {
    const auto [value, result] =
      decoder.decode(rexsapi::TValueType::FLOATING_POINT_MATRIX, enumValue, getNode(doc, "coded float32 matrix"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<rexsapi::TMatrix<double>>().m_Values.size() == 3);
    for (const auto& row : value.getValue<rexsapi::TMatrix<double>>().m_Values) {
      CHECK(row.size() == 3);
    }
  }

  SUBCASE("Decode string matrix")
  {
    const auto [value, result] =
      decoder.decode(rexsapi::TValueType::STRING_MATRIX, enumValue, getNode(doc, "string matrix"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    CHECK(value.getValue<rexsapi::TMatrix<std::string>>().m_Values.size() == 3);
    for (const auto& row : value.getValue<rexsapi::TMatrix<std::string>>().m_Values) {
      CHECK(row.size() == 2);
    }
  }

  SUBCASE("Decode array of integer arrays")
  {
    const auto [value, result] =
      decoder.decode(rexsapi::TValueType::ARRAY_OF_INTEGER_ARRAYS, enumValue, getNode(doc, "array of integer arrays"));
    CHECK(result == rexsapi::detail::TDecoderResult::SUCCESS);
    REQUIRE(value.getValue<std::vector<std::vector<int64_t>>>().size() == 3);
    CHECK(value.getValue<std::vector<std::vector<int64_t>>>()[0].size() == 3);
    CHECK(value.getValue<std::vector<std::vector<int64_t>>>()[1].size() == 2);
    CHECK(value.getValue<std::vector<std::vector<int64_t>>>()[2].size() == 1);
  }
}


TEST_CASE("Json value decoder error test")
{
  rexsapi::detail::TJsonValueDecoder decoder;
  std::optional<rexsapi::database::TEnumValues> enumValue;

  std::string document = R"(
  {
    "empty": { "integer": null },
    "boolean": { "boolean": "derk" },
    "float": { "floating_point": "puschel" },
    "integer": { "integer": "pi" },
    "string": { "string": "" },
    "enum": { "enum": "unknown enum" },
    "reference component": { "reference_component": "PR" },
    "file reference": { "file_reference": 4711 },
    "date time": { "date_time": "2023-02-31T11:11:11+00:00" },
    "float array": { "floating_point_array": ["a", "b", "c"] },
    "integer array": { "integer_array": ["a", "b", "c"] },
    "boolean array": { "boolean_array": ["true", "false", "false"] },
    "string array": { "string_array": [1, 2, 3] },
    "enum array": { "enum_array": [1, 2, 3] },
    "float matrix": { "floating_point_matrix": [[1.1, 1.2, 1.3], [2.1, 2.2], [3.1, 3.2, 3.3]] },
    "coded float matrix": { "floating_point_matrix_coded": { "code": "float64", "rows": 5, "columns": 3, "value": "AAAAAAAA8D8AAAAAAAAAQAAAAAAAAAhAAAAAAAAAEEAAAAAAAAAUQAAAAAAAABhAAAAAAAAAHEAAAAAAAAAgQAAAAAAAACJA" } },
    "string matrix": { "string_matrix": [["a", "b"], ["c"], ["e", "f"]] },
    "array of integer arrays": { "array_of_integer_arrays": [["a", "b", "c"], ["d", "e"], ["f"]] }
  }
  )";
  std::vector<uint8_t> buffer{document.begin(), document.end()};
  rexsapi::json doc = rexsapi::json::parse(buffer);

  SUBCASE("Decode no value")
  {
    CHECK(decoder.decode(rexsapi::TValueType::INTEGER, enumValue, getNode(doc, "empty")).second ==
          rexsapi::detail::TDecoderResult::NO_VALUE);
  }

  SUBCASE("Decode boolean")
  {
    CHECK(decoder.decode(rexsapi::TValueType::BOOLEAN, enumValue, getNode(doc, "boolean")).second ==
          rexsapi::detail::TDecoderResult::FAILURE);
  }

  SUBCASE("Decode integer")
  {
    CHECK(decoder.decode(rexsapi::TValueType::INTEGER, enumValue, getNode(doc, "integer")).second ==
          rexsapi::detail::TDecoderResult::FAILURE);
  }

  SUBCASE("Decode float")
  {
    CHECK(decoder.decode(rexsapi::TValueType::FLOATING_POINT, enumValue, getNode(doc, "float")).second ==
          rexsapi::detail::TDecoderResult::FAILURE);
  }

  SUBCASE("Decode string")
  {
    CHECK(decoder.decode(rexsapi::TValueType::STRING, enumValue, getNode(doc, "string")).second ==
          rexsapi::detail::TDecoderResult::FAILURE);
  }

  SUBCASE("Decode file reference")
  {
    CHECK(decoder.decode(rexsapi::TValueType::FILE_REFERENCE, enumValue, getNode(doc, "file reference")).second ==
          rexsapi::detail::TDecoderResult::FAILURE);
  }

  SUBCASE("Decode reference component")
  {
    CHECK(
      decoder.decode(rexsapi::TValueType::REFERENCE_COMPONENT, enumValue, getNode(doc, "reference component")).second ==
      rexsapi::detail::TDecoderResult::FAILURE);
  }

  SUBCASE("Decode enumValue")
  {
    enumValue = rexsapi::database::TEnumValues{{rexsapi::database::TEnumValue{"dip_lubrication", ""},
                                                rexsapi::database::TEnumValue{"injection_lubrication", ""}}};

    CHECK(decoder.decode(rexsapi::TValueType::ENUM, enumValue, getNode(doc, "enum")).second ==
          rexsapi::detail::TDecoderResult::FAILURE);
    CHECK(decoder.decode(rexsapi::TValueType::ENUM, {}, getNode(doc, "enum")).second ==
          rexsapi::detail::TDecoderResult::SUCCESS);
  }

  SUBCASE("Decode date time")
  {
    CHECK(decoder.decode(rexsapi::TValueType::DATE_TIME, enumValue, getNode(doc, "date time")).second ==
          rexsapi::detail::TDecoderResult::FAILURE);
  }

  SUBCASE("Decode integer array")
  {
    CHECK(decoder.decode(rexsapi::TValueType::INTEGER_ARRAY, enumValue, getNode(doc, "integer array")).second ==
          rexsapi::detail::TDecoderResult::FAILURE);
  }

  SUBCASE("Decode float array")
  {
    CHECK(decoder.decode(rexsapi::TValueType::FLOATING_POINT_ARRAY, enumValue, getNode(doc, "float array")).second ==
          rexsapi::detail::TDecoderResult::FAILURE);
  }

  SUBCASE("Decode boolean array")
  {
    CHECK(decoder.decode(rexsapi::TValueType::INTEGER_ARRAY, enumValue, getNode(doc, "boolean array")).second ==
          rexsapi::detail::TDecoderResult::WRONG_TYPE);
  }

  SUBCASE("Decode enum array")
  {
    CHECK(decoder.decode(rexsapi::TValueType::ENUM_ARRAY, enumValue, getNode(doc, "enum array")).second ==
          rexsapi::detail::TDecoderResult::FAILURE);
  }

  SUBCASE("Decode string array")
  {
    CHECK(decoder.decode(rexsapi::TValueType::STRING_ARRAY, enumValue, getNode(doc, "string array")).second ==
          rexsapi::detail::TDecoderResult::FAILURE);
  }

  SUBCASE("Decode float matrix")
  {
    CHECK(decoder.decode(rexsapi::TValueType::FLOATING_POINT_MATRIX, enumValue, getNode(doc, "float matrix")).second ==
          rexsapi::detail::TDecoderResult::FAILURE);
  }

  SUBCASE("Decode coded float matrix")
  {
    CHECK(decoder.decode(rexsapi::TValueType::FLOATING_POINT_MATRIX, enumValue, getNode(doc, "coded float matrix"))
            .second == rexsapi::detail::TDecoderResult::FAILURE);
  }

  SUBCASE("Decode string matrix")
  {
    CHECK(decoder.decode(rexsapi::TValueType::STRING_MATRIX, enumValue, getNode(doc, "string matrix")).second ==
          rexsapi::detail::TDecoderResult::FAILURE);
  }

  SUBCASE("Decode array of integer arrays")
  {
    CHECK(
      decoder.decode(rexsapi::TValueType::ARRAY_OF_INTEGER_ARRAYS, enumValue, getNode(doc, "array of integer arrays"))
        .second == rexsapi::detail::TDecoderResult::FAILURE);
  }
}
