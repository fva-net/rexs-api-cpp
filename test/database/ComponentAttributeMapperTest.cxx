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

#include <rexsapi/database/ComponentAttributeMapper.hxx>

#include <test/TestModelLoader.hxx>

#include <doctest.h>

TEST_CASE("Component attribute mapper")
{
  const auto& model = loadModel("1.4");

  SUBCASE("Check mappings")
  {
    std::vector<std::pair<std::string, std::string>> attributeMappings = {
      {"planet_carrier", "mass_of_component"},
      {"planet_carrier", "strut_inner_diameter"},
      {"profile_crowning", "profile_crowning_at_tip"}};

    rexsapi::database::detail::TComponentAttributeMapper mapper{model, std::move(attributeMappings)};

    CHECK(mapper.getAttributesForComponent("planet_carrier").size() == 2);
    CHECK(mapper.getAttributesForComponent("profile_crowning").size() == 1);
    CHECK(mapper.getAttributesForComponent("point_list").empty());
  }

  SUBCASE("Not existing attribute mapping")
  {
    std::vector<std::pair<std::string, std::string>> attributeMappings = {{"planet_carrier", "mass_of_component"},
                                                                          {"planet_carrier", "load_duration"},
                                                                          {"profile_twist", "not existing attribute"}};

    rexsapi::database::detail::TComponentAttributeMapper mapper{model, std::move(attributeMappings)};

    CHECK_THROWS_WITH((void)mapper.getAttributesForComponent("profile_twist"),
                      "attribute id=not existing attribute not found for component id=profile_twist");
  }
}
