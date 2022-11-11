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

#include <rexsapi/Unit.hxx>

#include <test/TestModelLoader.hxx>

#include <doctest.h>

TEST_CASE("Unit test")
{
  const auto& model = loadModel("1.4");

  SUBCASE("Regular unit")
  {
    rexsapi::TUnit unit{model.findUnitById(2)};
    CHECK_FALSE(unit.isCustomUnit());
    CHECK(unit.getName() == "mm");
    CHECK_FALSE(unit.getName() != "mm");

    CHECK(unit == model.findUnitById(2));
    CHECK_FALSE(unit == model.findUnitById(5));
    CHECK(unit != model.findUnitById(5));
    CHECK(unit == rexsapi::TUnit{model.findUnitById(2)});
    CHECK(unit == rexsapi::TUnit{"mm"});
    CHECK_FALSE(unit != rexsapi::TUnit{"mm"});
  }

  SUBCASE("Custom unit")
  {
    rexsapi::TUnit unit{"hutzli"};
    CHECK(unit.isCustomUnit());
    CHECK(unit.getName() == "hutzli");
    CHECK_FALSE(unit.getName() != "hutzli");
    CHECK_FALSE(unit == model.findUnitById(56));
    CHECK(unit != model.findUnitById(56));
    CHECK(rexsapi::TUnit{"kg"} == model.findUnitById(56));
    CHECK(rexsapi::TUnit{"kg"} == rexsapi::TUnit{model.findUnitById(56)});
    CHECK(unit == rexsapi::TUnit{"hutzli"});
    CHECK_FALSE(unit != rexsapi::TUnit{"hutzli"});
  }
}
