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

#include <rexsapi/database/EnumValues.hxx>

#include <doctest.h>


TEST_CASE("Enum values test")
{
  std::vector<rexsapi::database::TEnumValue> values;
  values.emplace_back(rexsapi::database::TEnumValue{"both_directions", "Both directions"});
  values.emplace_back(rexsapi::database::TEnumValue{"negative", "Negative u-direction"});
  values.emplace_back(rexsapi::database::TEnumValue{"no_direction", "No direction"});
  values.emplace_back(rexsapi::database::TEnumValue{"positive", "Positive u-direction"});

  rexsapi::database::TEnumValues enumValues{std::move(values)};

  SUBCASE("Check existing value")
  {
    CHECK(enumValues.check("both_directions"));
    CHECK(enumValues.check("negative"));
    CHECK(enumValues.check("no_direction"));
    CHECK(enumValues.check("positive"));
  }

  SUBCASE("Check non existing value")
  {
    CHECK_FALSE(enumValues.check("this_is_no_value"));
    CHECK_FALSE(enumValues.check(""));
  }
}
