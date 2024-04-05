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

#include <rexsapi/Types.hxx>

#include <regex>

#include <doctest.h>


TEST_CASE("Relation type test")
{
  SUBCASE("From string")
  {
    CHECK(rexsapi::relationTypeFromString("assembly") == rexsapi::TRelationType::ASSEMBLY);
    CHECK(rexsapi::relationTypeFromString("central_shaft") == rexsapi::TRelationType::CENTRAL_SHAFT);
    CHECK(rexsapi::relationTypeFromString("connection") == rexsapi::TRelationType::CONNECTION);
    CHECK(rexsapi::relationTypeFromString("contact") == rexsapi::TRelationType::CONTACT);
    CHECK(rexsapi::relationTypeFromString("coupling") == rexsapi::TRelationType::COUPLING);
    CHECK(rexsapi::relationTypeFromString("flank") == rexsapi::TRelationType::FLANK);
    CHECK(rexsapi::relationTypeFromString("manufacturing_step") == rexsapi::TRelationType::MANUFACTURING_STEP);
    CHECK(rexsapi::relationTypeFromString("ordered_assembly") == rexsapi::TRelationType::ORDERED_ASSEMBLY);
    CHECK(rexsapi::relationTypeFromString("ordered_reference") == rexsapi::TRelationType::ORDERED_REFERENCE);
    CHECK(rexsapi::relationTypeFromString("planet_carrier_shaft") == rexsapi::TRelationType::PLANET_CARRIER_SHAFT);
    CHECK(rexsapi::relationTypeFromString("planet_pin") == rexsapi::TRelationType::PLANET_PIN);
    CHECK(rexsapi::relationTypeFromString("planet_shaft") == rexsapi::TRelationType::PLANET_SHAFT);
    CHECK(rexsapi::relationTypeFromString("reference") == rexsapi::TRelationType::REFERENCE);
    CHECK(rexsapi::relationTypeFromString("side") == rexsapi::TRelationType::SIDE);
    CHECK(rexsapi::relationTypeFromString("stage") == rexsapi::TRelationType::STAGE);
    CHECK(rexsapi::relationTypeFromString("stage_gear_data") == rexsapi::TRelationType::STAGE_GEAR_DATA);
    CHECK_THROWS(rexsapi::relationTypeFromString("unknown relation type"));
  }

  SUBCASE("To string")
  {
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::ASSEMBLY) == "assembly");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::CENTRAL_SHAFT) == "central_shaft");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::CONNECTION) == "connection");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::CONTACT) == "contact");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::COUPLING) == "coupling");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::FLANK) == "flank");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::MANUFACTURING_STEP) == "manufacturing_step");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::ORDERED_ASSEMBLY) == "ordered_assembly");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::ORDERED_REFERENCE) == "ordered_reference");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::PLANET_CARRIER_SHAFT) == "planet_carrier_shaft");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::PLANET_PIN) == "planet_pin");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::PLANET_SHAFT) == "planet_shaft");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::REFERENCE) == "reference");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::SIDE) == "side");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::STAGE) == "stage");
    CHECK(rexsapi::toRelationTypeString(rexsapi::TRelationType::STAGE_GEAR_DATA) == "stage_gear_data");
  }
}

TEST_CASE("Relation role test")
{
  SUBCASE("From string")
  {
    CHECK(rexsapi::relationRoleFromString("assembly") == rexsapi::TRelationRole::ASSEMBLY);
    CHECK(rexsapi::relationRoleFromString("gear") == rexsapi::TRelationRole::GEAR);
    CHECK(rexsapi::relationRoleFromString("gear_1") == rexsapi::TRelationRole::GEAR_1);
    CHECK(rexsapi::relationRoleFromString("gear_2") == rexsapi::TRelationRole::GEAR_2);
    CHECK(rexsapi::relationRoleFromString("inner_part") == rexsapi::TRelationRole::INNER_PART);
    CHECK(rexsapi::relationRoleFromString("left") == rexsapi::TRelationRole::LEFT);
    CHECK(rexsapi::relationRoleFromString("manufacturing_settings") == rexsapi::TRelationRole::MANUFACTURING_SETTINGS);
    CHECK(rexsapi::relationRoleFromString("origin") == rexsapi::TRelationRole::ORIGIN);
    CHECK(rexsapi::relationRoleFromString("outer_part") == rexsapi::TRelationRole::OUTER_PART);
    CHECK(rexsapi::relationRoleFromString("part") == rexsapi::TRelationRole::PART);
    CHECK(rexsapi::relationRoleFromString("planetary_stage") == rexsapi::TRelationRole::PLANETARY_STAGE);
    CHECK(rexsapi::relationRoleFromString("referenced") == rexsapi::TRelationRole::REFERENCED);
    CHECK(rexsapi::relationRoleFromString("right") == rexsapi::TRelationRole::RIGHT);
    CHECK(rexsapi::relationRoleFromString("shaft") == rexsapi::TRelationRole::SHAFT);
    CHECK(rexsapi::relationRoleFromString("side_1") == rexsapi::TRelationRole::SIDE_1);
    CHECK(rexsapi::relationRoleFromString("side_2") == rexsapi::TRelationRole::SIDE_2);
    CHECK(rexsapi::relationRoleFromString("stage") == rexsapi::TRelationRole::STAGE);
    CHECK(rexsapi::relationRoleFromString("stage_gear_data") == rexsapi::TRelationRole::STAGE_GEAR_DATA);
    CHECK(rexsapi::relationRoleFromString("tool") == rexsapi::TRelationRole::TOOL);
    CHECK(rexsapi::relationRoleFromString("workpiece") == rexsapi::TRelationRole::WORKPIECE);
    CHECK_THROWS(rexsapi::relationRoleFromString("unknown relation role"));
  }

  SUBCASE("To string")
  {
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::ASSEMBLY) == "assembly");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::GEAR) == "gear");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::GEAR_1) == "gear_1");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::GEAR_2) == "gear_2");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::INNER_PART) == "inner_part");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::LEFT) == "left");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::MANUFACTURING_SETTINGS) == "manufacturing_settings");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::ORIGIN) == "origin");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::OUTER_PART) == "outer_part");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::PART) == "part");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::PLANETARY_STAGE) == "planetary_stage");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::REFERENCED) == "referenced");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::RIGHT) == "right");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::SHAFT) == "shaft");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::SIDE_1) == "side_1");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::SIDE_2) == "side_2");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::STAGE) == "stage");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::STAGE_GEAR_DATA) == "stage_gear_data");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::TOOL) == "tool");
    CHECK(rexsapi::toRelationRoleString(rexsapi::TRelationRole::WORKPIECE) == "workpiece");
  }
}

TEST_CASE("Bool test")
{
  const rexsapi::Bool bTrue{true};
  const rexsapi::Bool bFalse{false};

  SUBCASE("Create")
  {
    CHECK(bTrue);
    CHECK_FALSE(bFalse);
  }

  SUBCASE("Compare")
  {
    CHECK_FALSE(bTrue == bFalse);
    CHECK_FALSE(bFalse == bTrue);
    CHECK(bTrue == true);
    CHECK(bFalse == false);
    CHECK(true == bTrue);
    CHECK(false == bFalse);

    CHECK(bTrue != bFalse);
    CHECK_FALSE(bTrue != true);
    CHECK_FALSE(bFalse != false);
    CHECK_FALSE(true != bTrue);
    CHECK_FALSE(false != bFalse);
  }

  SUBCASE("Operators")
  {
    CHECK(*bTrue);
    CHECK_FALSE(*bFalse);
  }
}

TEST_CASE("Datetime test")
{
  const std::regex reg_expr(R"(^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})[+-](\d{2}):(\d{2})$)");
  std::smatch match;

  SUBCASE("Parse")
  {
    const rexsapi::TDatetime dt{"2023-03-28T13:49:36+02:00"};
    using namespace date;
    CHECK(dt.asTimepoint() == date::sys_days{2023_y / mar / 28} + std::chrono::hours{11} + std::chrono::minutes{49} +
                                std::chrono::seconds{36});
    CHECK(dt.asUTCString() == "2023-03-28T11:49:36+00:00");
    const auto s = dt.asLocaleString();
    CHECK(std::regex_match(s, match, reg_expr));
  }

  SUBCASE("Equality")
  {
    const rexsapi::TDatetime dt{"2023-03-28T13:49:36+02:00"};
    CHECK(dt == dt);
    const rexsapi::TDatetime dt1{"2022-03-28T13:49:36+02:00"};
    CHECK_FALSE(dt == dt1);
    CHECK_FALSE(dt1 == dt);
  }

  SUBCASE("Now")
  {
    const rexsapi::TDatetime dt = rexsapi::TDatetime::now();
    const auto s = dt.asLocaleString();
    CHECK(std::regex_match(s, match, reg_expr));
  }

  SUBCASE("Illegal date")
  {
    CHECK_THROWS(rexsapi::TDatetime{"2023-02-31T13:49:36+02:00"});
  }
}
