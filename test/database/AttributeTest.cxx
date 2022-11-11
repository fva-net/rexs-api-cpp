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

#include <rexsapi/database/Attribute.hxx>

#include <doctest.h>


TEST_CASE("Database attribute test")
{
  rexsapi::database::TUnit unit{58, "deg"};

  SUBCASE("Create enum attribute")
  {
    std::vector<rexsapi::database::TEnumValue> values;
    values.emplace_back(rexsapi::database::TEnumValue{"both_directions", "Both directions"});
    values.emplace_back(rexsapi::database::TEnumValue{"negative", "Negative u-direction"});
    values.emplace_back(rexsapi::database::TEnumValue{"no_direction", "No direction"});
    values.emplace_back(rexsapi::database::TEnumValue{"positive", "Positive u-direction"});
    rexsapi::database::TEnumValues enumValues{std::move(values)};

    rexsapi::database::TAttribute attribute{
      "axial_force_absorption", "Support of axial loads", rexsapi::TValueType::ENUM, unit, "", {}, enumValues};

    CHECK(attribute.getAttributeId() == "axial_force_absorption");
    CHECK(attribute.getName() == "Support of axial loads");
    CHECK(attribute.getValueType() == rexsapi::TValueType::ENUM);
    CHECK(attribute.getUnit() == unit);
    CHECK(attribute.getSymbol().empty());
    CHECK_FALSE(attribute.getInterval());
    REQUIRE(attribute.getEnums());
    CHECK(attribute.getEnums()->check("negative"));
  }

  SUBCASE("Create enum attribute without enum values")
  {
    CHECK_THROWS_WITH(rexsapi::database::TAttribute("axial_force_absorption", "Support of axial loads",
                                                    rexsapi::TValueType::ENUM, unit, "", {}, {}),
                      "enum of attribute id=axial_force_absorption does not have any values");
  }

  SUBCASE("Create non-enum attribute")
  {
    rexsapi::database::TInterval interval{{0, rexsapi::database::TIntervalType::OPEN},
                                          {180, rexsapi::database::TIntervalType::OPEN}};

    rexsapi::database::TAttribute attribute{
      "chamfer_angle_worm_wheel", "Chamfer ange", rexsapi::TValueType::FLOATING_POINT, unit, "ϑ", interval, {}};

    CHECK(attribute.getAttributeId() == "chamfer_angle_worm_wheel");
    CHECK(attribute.getName() == "Chamfer ange");
    CHECK(attribute.getValueType() == rexsapi::TValueType::FLOATING_POINT);
    CHECK(attribute.getUnit() == unit);
    CHECK(attribute.getSymbol() == "ϑ");
    REQUIRE(attribute.getInterval());
    CHECK(attribute.getInterval()->check(90));
    CHECK_FALSE(attribute.getInterval()->check(290));
    CHECK_FALSE(attribute.getEnums());
  }
}
