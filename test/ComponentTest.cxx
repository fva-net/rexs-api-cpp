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

#include <rexsapi/Component.hxx>

#include <test/TestModelLoader.hxx>

#include <doctest.h>

TEST_CASE("Component test")
{
  const auto& dbModel = loadModel("1.4");
  const auto& dbComponent = dbModel.findComponentById("bevel_gear");
  rexsapi::TAttributes attributes{
    rexsapi::TAttribute{dbComponent.findAttributeById("face_angle"), rexsapi::TValue{73.2}},
    rexsapi::TAttribute{dbComponent.findAttributeById("mass_of_component"), rexsapi::TValue{3.75}}};

  SUBCASE("Create")
  {
    rexsapi::TComponent component{4711, dbComponent, "Kegelrad", attributes};
    CHECK(component.getExternalId() == std::numeric_limits<uint64_t>::max());
    CHECK(component.getInternalId() == 4711);
    CHECK(component.getId() == 4711);
    CHECK(component.getName() == "Kegelrad");
    CHECK(component.getType() == "bevel_gear");
    REQUIRE(component.getAttributes().size() == 2);
    CHECK(component.getAttributes()[0].getAttributeId() == "face_angle");
    CHECK(component.getAttributes()[1].getAttributeId() == "mass_of_component");

    rexsapi::TComponent componentCopy{component};
    CHECK(componentCopy.getExternalId() == std::numeric_limits<uint64_t>::max());
    CHECK(componentCopy.getInternalId() == 4711);
    CHECK(componentCopy.getId() == 4711);
    CHECK(componentCopy.getName() == "Kegelrad");
    CHECK(componentCopy.getType() == "bevel_gear");
    REQUIRE(componentCopy.getAttributes().size() == 2);
    CHECK(componentCopy.getAttributes()[0].getAttributeId() == "face_angle");
    CHECK(componentCopy.getAttributes()[1].getAttributeId() == "mass_of_component");
  }

  SUBCASE("Create with external id")
  {
    rexsapi::TComponent component{815, 4711, dbComponent, "Kegelrad", attributes};
    CHECK(component.getExternalId() == 815);
    CHECK(component.getInternalId() == 4711);
    CHECK(component.getId() == 815);
    CHECK(component.getName() == "Kegelrad");
    CHECK(component.getType() == "bevel_gear");
    REQUIRE(component.getAttributes().size() == 2);
    CHECK(component.getAttributes()[0].getAttributeId() == "face_angle");
    CHECK(component.getAttributes()[1].getAttributeId() == "mass_of_component");

    rexsapi::TComponent componentCopy{component};
    CHECK(componentCopy.getExternalId() == 815);
    CHECK(componentCopy.getInternalId() == 4711);
    CHECK(componentCopy.getId() == 815);
    CHECK(componentCopy.getName() == "Kegelrad");
    CHECK(componentCopy.getType() == "bevel_gear");
    REQUIRE(componentCopy.getAttributes().size() == 2);
    CHECK(componentCopy.getAttributes()[0].getAttributeId() == "face_angle");
    CHECK(componentCopy.getAttributes()[1].getAttributeId() == "mass_of_component");
  }
}
