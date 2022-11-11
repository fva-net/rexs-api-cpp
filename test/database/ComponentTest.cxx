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

#include <rexsapi/database/Component.hxx>

#include <doctest.h>


TEST_CASE("Component test")
{
  const rexsapi::database::TUnit unitDeg{58, "deg"};
  const rexsapi::database::TUnit unitMum{11, "mum"};

  rexsapi::database::TAttribute attribute1{
    "chamfer_angle_worm_wheel", "Chamfer ange", rexsapi::TValueType::FLOATING_POINT, unitDeg, "ϑ", {}, {}};
  rexsapi::database::TAttribute attribute2{"arithmetic_average_roughness_root",
                                           "Arithmetic average roughness root",
                                           rexsapi::TValueType::FLOATING_POINT,
                                           unitMum,
                                           "R_aF",
                                           {},
                                           {}};

  SUBCASE("Create component")
  {
    std::vector<std::reference_wrapper<const rexsapi::database::TAttribute>> attributes;
    attributes.emplace_back(std::ref(attribute1));
    attributes.emplace_back(std::ref(attribute2));

    rexsapi::database::TComponent component{"cylindrical_gear", "Cylindrical gear", std::move(attributes)};

    CHECK(component.getComponentId() == "cylindrical_gear");
    CHECK(component.getName() == "Cylindrical gear");
    auto atts = component.getAttributes();
    CHECK(atts.size() == 2);
    REQUIRE(component.hasAttribute("arithmetic_average_roughness_root"));
    const auto& attribute = component.findAttributeById("arithmetic_average_roughness_root");
    CHECK(attribute.getAttributeId() == "arithmetic_average_roughness_root");
    CHECK_THROWS_WITH((void)component.findAttributeById("not-available-attribute"),
                      "component id=cylindrical_gear does not contain attribute id=not-available-attribute");
    CHECK_FALSE(component.hasAttribute("not-available-attribute"));
  }
}
