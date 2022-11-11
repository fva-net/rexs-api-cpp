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

#include <rexsapi/database/Unit.hxx>

#include <doctest.h>


TEST_CASE("Database unit test")
{
  SUBCASE("Create")
  {
    rexsapi::database::TUnit unit{4711, "kg"};
    CHECK(unit.getId() == 4711);
    CHECK(unit.getName() == "kg");
  }

  SUBCASE("Compare")
  {
    rexsapi::database::TUnit unit{47, "N / (mm s^0.5 K)"};
    CHECK(unit.compare("N / (mm s^0.5 K)"));
    CHECK_FALSE(unit.compare("N / mm s^0.5 K"));
    CHECK_FALSE(unit.compare(""));

    CHECK(unit == rexsapi::database::TUnit{47, "N / (mm s^0.5 K)"});
    CHECK(unit != rexsapi::database::TUnit{37, "N / (mm mum)"});
  }
}
