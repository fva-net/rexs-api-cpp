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

#include <rexsapi/ValidityChecker.hxx>

#include <test/TestModelLoader.hxx>

#include <doctest.h>


TEST_CASE("Validity checker test")
{
  rexsapi::database::TInterval interval{
    rexsapi::database::TIntervalEndpoint{0, rexsapi::database::TIntervalType::CLOSED},
    rexsapi::database::TIntervalEndpoint{10, rexsapi::database::TIntervalType::CLOSED}};

  const auto& dbModel = loadModel("1.4");

  SUBCASE("double with interval")
  {
    const auto& attribute = dbModel.findAttributeById("temperature_lubricant");
    CHECK(rexsapi::detail::TValidityChecker::check(attribute, rexsapi::TValue{73.2}));
    CHECK_FALSE(rexsapi::detail::TValidityChecker::check(attribute, rexsapi::TValue{-300.0}));
  }

  SUBCASE("double without interval")
  {
    const auto& attribute = dbModel.findAttributeById("actual_tangential_load_agma_925_a03");
    CHECK(rexsapi::detail::TValidityChecker::check(attribute, rexsapi::TValue{73.2}));
    CHECK(rexsapi::detail::TValidityChecker::check(attribute, rexsapi::TValue{-300.0}));
  }

  SUBCASE("double array")
  {
    const auto& attribute = dbModel.findAttributeById("central_film_thickness_at_calculation_points_agma_925_a03");
    CHECK(
      rexsapi::detail::TValidityChecker::check(attribute, rexsapi::TValue{std::vector<double>{47.11, 0.815, 57.4}}));
    CHECK_FALSE(
      rexsapi::detail::TValidityChecker::check(attribute, rexsapi::TValue{std::vector<double>{47.11, 0.815, -57.4}}));
  }

  SUBCASE("double matrix")
  {
    rexsapi::database::TAttribute attribute{"double matrix attribute",
                                            "dm",
                                            rexsapi::TValueType::FLOATING_POINT_MATRIX,
                                            rexsapi::database::TUnit{30, "kW"},
                                            "",
                                            interval,
                                            {}};
    CHECK(rexsapi::detail::TValidityChecker::check(
      attribute,
      rexsapi::TValue{rexsapi::TMatrix<double>{std::vector<std::vector<double>>{{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}}}}));
    CHECK_FALSE(rexsapi::detail::TValidityChecker::check(
      attribute,
      rexsapi::TValue{rexsapi::TMatrix<double>{std::vector<std::vector<double>>{{{1, 2, 3}, {4, 57, 6}, {7, 8, 9}}}}}));
  }

  SUBCASE("intreger matrix")
  {
    rexsapi::database::TAttribute attribute{"integer matrix attribute",
                                            "im",
                                            rexsapi::TValueType::INTEGER_MATRIX,
                                            rexsapi::database::TUnit{30, "kW"},
                                            "",
                                            interval,
                                            {}};
    CHECK(rexsapi::detail::TValidityChecker::check(
      attribute, rexsapi::TValue{
                   rexsapi::TMatrix<int64_t>{std::vector<std::vector<int64_t>>{{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}}}}));
    CHECK_FALSE(rexsapi::detail::TValidityChecker::check(
      attribute, rexsapi::TValue{
                   rexsapi::TMatrix<int64_t>{std::vector<std::vector<int64_t>>{{{1, 2, 3}, {4, 57, 6}, {7, 8, 9}}}}}));
  }

  SUBCASE("integer with interval")
  {
    const auto& attribute = dbModel.findAttributeById("gear_shift_index");
    CHECK(rexsapi::detail::TValidityChecker::check(attribute, rexsapi::TValue{15}));
    CHECK_FALSE(rexsapi::detail::TValidityChecker::check(attribute, rexsapi::TValue{0}));
  }

  SUBCASE("integer array")
  {
    rexsapi::database::TAttribute attribute{"integer array attribute",
                                            "ia",
                                            rexsapi::TValueType::INTEGER_ARRAY,
                                            rexsapi::database::TUnit{30, "kW"},
                                            "",
                                            interval,
                                            {}};
    CHECK(rexsapi::detail::TValidityChecker::check(attribute, rexsapi::TValue{std::vector<int64_t>{1, 2, 3}}));
    CHECK_FALSE(rexsapi::detail::TValidityChecker::check(attribute, rexsapi::TValue{std::vector<int64_t>{1, 57, 3}}));
  }

  SUBCASE("array of integer arrays")
  {
    rexsapi::database::TAttribute attribute{"array of integer array attribute",
                                            "aia",
                                            rexsapi::TValueType::ARRAY_OF_INTEGER_ARRAYS,
                                            rexsapi::database::TUnit{30, "kW"},
                                            "",
                                            interval,
                                            {}};
    CHECK(rexsapi::detail::TValidityChecker::check(
      attribute, rexsapi::TValue{std::vector<std::vector<int64_t>>{{{1, 2, 3}, {4, 5, 6}}}}));
    CHECK_FALSE(rexsapi::detail::TValidityChecker::check(
      attribute, rexsapi::TValue{std::vector<std::vector<int64_t>>{{{1, 2, 3}, {4, 57, 6}}}}));
  }

  SUBCASE("enum")
  {
    const auto& attribute = dbModel.findAttributeById("axial_clearance_class");
    CHECK(rexsapi::detail::TValidityChecker::check(attribute, rexsapi::TValue{"C1"}));
    CHECK_FALSE(rexsapi::detail::TValidityChecker::check(attribute, rexsapi::TValue{"C4711"}));
  }

  SUBCASE("enum array")
  {
    const auto& attribute = dbModel.findAttributeById("element_types");
    CHECK(rexsapi::detail::TValidityChecker::check(attribute,
                                                   rexsapi::TValue{std::vector<std::string>{"line2", "pyramid12"}}));
    CHECK_FALSE(rexsapi::detail::TValidityChecker::check(
      attribute, rexsapi::TValue{std::vector<std::string>{"line2", "PUSCHEL", "pyramid12"}}));
  }

  SUBCASE("bool")
  {
    const auto& attribute = dbModel.findAttributeById("account_for_gravity");
    CHECK(rexsapi::detail::TValidityChecker::check(attribute, rexsapi::TValue{true}));
    CHECK(rexsapi::detail::TValidityChecker::check(attribute, rexsapi::TValue{false}));
  }
}
