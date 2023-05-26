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

#include <rexsapi/ConversionHelper.hxx>

#include <iomanip>
#include <iostream>
#include <limits>
#include <regex>

#include <doctest.h>


TEST_CASE("Conversion unsigned integer test")
{
  SUBCASE("Convert success")
  {
    CHECK(rexsapi::convertToUint64("4711") == 4711);
    CHECK(rexsapi::convertToUint64("  4711") == 4711);
    CHECK(rexsapi::convertToUint64("0") == 0);
    CHECK(rexsapi::convertToUint64(std::to_string(std::numeric_limits<uint64_t>::max())) ==
          std::numeric_limits<uint64_t>::max());
    CHECK(rexsapi::convertToUint64(std::to_string(std::numeric_limits<uint64_t>::min())) ==
          std::numeric_limits<uint64_t>::min());
  }

  SUBCASE("Conversion fail")
  {
    CHECK_THROWS_WITH(rexsapi::convertToUint64("a4711"),
                      "cannot convert string 'a4711' to unsigned integer: invalid argument");
    CHECK_THROWS_WITH(rexsapi::convertToUint64("4711puschel"),
                      "cannot convert string to unsigned integer: 4711puschel");
    CHECK_THROWS_WITH(rexsapi::convertToUint64("-4711"), "cannot convert string to unsigned integer: -4711");
    CHECK_THROWS_WITH(rexsapi::convertToUint64(std::to_string(std::numeric_limits<uint64_t>::max()) + "1"),
                      "cannot convert string '184467440737095516151' to unsigned integer: out of range");
  }
}

TEST_CASE("Conversion integer test")
{
  SUBCASE("Convert success")
  {
    CHECK(rexsapi::convertToInt64("4711") == 4711);
    CHECK(rexsapi::convertToInt64("  4711") == 4711);
    CHECK(rexsapi::convertToInt64("-4711") == -4711);
    CHECK(rexsapi::convertToInt64("  -4711") == -4711);
    CHECK(rexsapi::convertToInt64("0") == 0);
    CHECK(rexsapi::convertToInt64(std::to_string(std::numeric_limits<int64_t>::max())) ==
          std::numeric_limits<int64_t>::max());
    CHECK(rexsapi::convertToInt64(std::to_string(std::numeric_limits<int64_t>::min())) ==
          std::numeric_limits<int64_t>::min());
  }

  SUBCASE("Conversion fail")
  {
    CHECK_THROWS_WITH(rexsapi::convertToInt64("a4711"), "cannot convert string 'a4711' to integer: invalid argument");
    CHECK_THROWS_WITH(rexsapi::convertToInt64("4711puschel"), "cannot convert string to integer: 4711puschel");
    CHECK_THROWS_WITH(rexsapi::convertToInt64(std::to_string(std::numeric_limits<int64_t>::max()) + "1"),
                      "cannot convert string '92233720368547758071' to integer: out of range");
  }
}

TEST_CASE("Conversion double test")
{
  SUBCASE("Convert success")
  {
    CHECK(rexsapi::convertToDouble("47.11") == doctest::Approx(47.11));
    CHECK(rexsapi::convertToDouble("  47.11") == doctest::Approx(47.11));
    CHECK(rexsapi::convertToDouble("-47.11") == doctest::Approx(-47.11));
    CHECK(rexsapi::convertToDouble("  -47.11") == doctest::Approx(-47.11));
    CHECK(rexsapi::convertToDouble("0.0") == doctest::Approx(0.0));
    CHECK(rexsapi::convertToDouble(std::to_string(std::numeric_limits<double>::max())) ==
          doctest::Approx(std::numeric_limits<double>::max()));
    CHECK(rexsapi::convertToDouble(std::to_string(std::numeric_limits<double>::min())) ==
          doctest::Approx(std::numeric_limits<double>::min()));
    const double d = rexsapi::convertToDouble("37.849999999999");
    const auto s = rexsapi::format(d);
    CHECK(s == "37.849999999999");
  }

  SUBCASE("Conversion fail")
  {
    CHECK_THROWS_WITH(rexsapi::convertToDouble("a47.11"), "cannot convert string 'a47.11' to double: invalid argument");
    CHECK_THROWS_WITH(rexsapi::convertToDouble("47.11puschel"), "cannot convert string to double: 47.11puschel");
  }
}

TEST_CASE("Time helper")
{
  SUBCASE("ISO8601 date")
  {
    const auto s = rexsapi::getTimeStringISO8601(std::chrono::system_clock::now());
    std::regex reg_expr(R"(^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})[+-](\d{2}):(\d{2})$)");
    std::smatch match;
    CHECK(std::regex_match(s, match, reg_expr));
  }
}

TEST_CASE("String helper")
{
  SUBCASE("To upper")
  {
    const std::string s{"Some lower and some Upper ChaRacter"};
    CHECK(rexsapi::toupper(s) == "SOME LOWER AND SOME UPPER CHARACTER");
  }
}

TEST_CASE("Format helper")
{
  SUBCASE("Small double")
  {
    CHECK(rexsapi::format(47.11) == "47.11");
    CHECK(rexsapi::format(0.0) == "0.0");
    CHECK(rexsapi::format(17.0) == "17.0");
  }
  SUBCASE("Big double")
  {
    CHECK(rexsapi::format(std::numeric_limits<double>::max()) == "1.79769313486232E+308");
    CHECK(rexsapi::format(1.0e-10) == "1E-10");
  }
  SUBCASE("15 significant digits")
  {
    // clang-format off
    CHECK(rexsapi::format( 37.8499999999994)   ==  "37.8499999999994");
    CHECK(rexsapi::format( 37.84999999999994)  ==  "37.8499999999999");
    CHECK(rexsapi::format( 37.849999999999994) ==  "37.85");
    CHECK(rexsapi::format(137.849999999994)    == "137.849999999994");
    CHECK(rexsapi::format(137.8499999999994)   == "137.849999999999");
    CHECK(rexsapi::format(137.8499999999999)   == "137.85");
    // clang-format on
  }
}
