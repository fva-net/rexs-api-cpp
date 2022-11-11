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

#include <rexsapi/Attribute.hxx>

#include <test/TestModelLoader.hxx>

#include <doctest.h>

TEST_CASE("Attribute test")
{
  const auto& dbModel = loadModel("1.4");

  SUBCASE("Copy")
  {
    rexsapi::TAttribute attribute{dbModel.findAttributeById("viscosity_at_100_degree_celsius"), rexsapi::TValue{5.5}};
    CHECK_FALSE(attribute.isCustomAttribute());
    auto copiedAttribute = attribute;
    CHECK_FALSE(copiedAttribute.isCustomAttribute());
    CHECK(copiedAttribute.getName() == "Viscosity at 100°C");
    CHECK(copiedAttribute.getAttributeId() == attribute.getAttributeId());
    CHECK(copiedAttribute.getUnit() == attribute.getUnit());
    CHECK(copiedAttribute.getValue() == attribute.getValue());
    CHECK(copiedAttribute.hasValue());
  }

  SUBCASE("Custom attribute")
  {
    rexsapi::TAttribute attribute{"custom_load_duration_fraction", rexsapi::TUnit{"%"}, rexsapi::TValueType::STRING,
                                  rexsapi::TValue{"30"}};
    CHECK(attribute.isCustomAttribute());
    CHECK(attribute.getAttributeId() == "custom_load_duration_fraction");
    CHECK(attribute.getName() == "custom_load_duration_fraction");
    CHECK(attribute.getUnit() == rexsapi::TUnit{"%"});
    CHECK(attribute.getValueType() == rexsapi::TValueType::STRING);
    CHECK(attribute.hasValue());
  }

  SUBCASE("Custom attribute failure")
  {
    CHECK_THROWS(rexsapi::TAttribute{"", rexsapi::TUnit{"%"}, rexsapi::TValueType::STRING, rexsapi::TValue{"30"}});
  }
}
