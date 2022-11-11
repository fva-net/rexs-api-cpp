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

#include <rexsapi/JsonValueDecoder.hxx>
#include <rexsapi/ModelHelper.hxx>
#include <rexsapi/XMLValueDecoder.hxx>

#include <test/TestHelper.hxx>
#include <test/TestModelLoader.hxx>

#include <doctest.h>

TEST_CASE("Model helper json test")
{
  const auto& dbModel = loadModel("1.4");
  rexsapi::detail::TModelHelper<rexsapi::detail::TJsonValueDecoder> helper{rexsapi::TMode::STRICT_MODE};
  rexsapi::TResult result;
  const std::string context{"test"};

  SUBCASE("Check custom")
  {
    CHECK_FALSE(helper.checkCustom(result, context, "account_for_gravity", 42, dbModel.findComponentById("gear_unit")));
    CHECK(result);
    CHECK(helper.checkCustom(result, context, "diameter_of_helix_modification", 42,
                             dbModel.findComponentById("gear_unit")));
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 1);
    CHECK(result.getErrors()[0].getMessage() ==
          "test: attribute id=diameter_of_helix_modification is not part of component gear_unit id=42");

    result.reset();
    CHECK(helper.checkCustom(result, context, "custom_my_attribute", 42, dbModel.findComponentById("gear_unit")));
  }

  SUBCASE("Get value success")
  {
    rexsapi::json node = rexsapi::json::parse(R"({
      "id": "account_for_gravity", "unit": "none", "boolean": true
})");
    auto ret = helper.getValue(result, context, "account_for_gravity", 42,
                               dbModel.findAttributeById("account_for_gravity"), node);
    CHECK(ret.getValue<bool>());
    CHECK(result);
  }

  SUBCASE("Get value failure")
  {
    rexsapi::json node = rexsapi::json::parse(R"({
      "id": "account_for_gravity", "unit": "none", "integer": 42
})");
    auto ret = helper.getValue(result, context, "account_for_gravity", 42,
                               dbModel.findAttributeById("account_for_gravity"), node);
    CHECK(ret.isEmpty());
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 1);
    CHECK(result.getErrors()[0].getMessage() ==
          "test: value of attribute id=account_for_gravity of component id=42 does not have the correct value type");
  }

  SUBCASE("Get empty value")
  {
    rexsapi::json node = rexsapi::json::parse(R"({
      "id": "account_for_gravity", "unit": "none", "boolean": null
})");
    auto ret = helper.getValue(result, context, "account_for_gravity", 42,
                               dbModel.findAttributeById("account_for_gravity"), node);
    CHECK(ret.isEmpty());
    CHECK(result);
    REQUIRE(result.getErrors().size() == 1);
    CHECK(result.getErrors()[0].getMessage() ==
          "test: value of attribute id=account_for_gravity of component id=42 is empty");
  }

  SUBCASE("Get value range failure")
  {
    rexsapi::json node = rexsapi::json::parse(R"({
      "id": "shear_modulus", "unit": "N / mm^2", "floating_point": -42.0
})");
    auto ret = helper.getValue(result, context, "shear_modulus", 42, dbModel.findAttributeById("shear_modulus"), node);
    CHECK(ret.getValue<double>() == doctest::Approx{-42.0});
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 1);
    CHECK(result.getErrors()[0].getMessage() ==
          "test: value is out of range for attribute id=shear_modulus of component id=42");
    CHECK(result.getErrors()[0].isError());
  }

  SUBCASE("Get value success with explicit type")
  {
    rexsapi::json node = rexsapi::json::parse(R"({
      "id": "account_for_gravity", "unit": "none", "boolean": true
})");
    auto ret = helper.getValue(result, rexsapi::TValueType::BOOLEAN, context, "account_for_gravity", 42, node);
    CHECK(ret.getValue<bool>());
    CHECK(result);
  }

  SUBCASE("Get value failure with explicit type")
  {
    rexsapi::json node = rexsapi::json::parse(R"({
      "id": "account_for_gravity", "unit": "none", "integer": 42
})");
    auto ret = helper.getValue(result, rexsapi::TValueType::BOOLEAN, context, "account_for_gravity", 42, node);
    CHECK(ret.isEmpty());
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 1);
    CHECK(result.getErrors()[0].getMessage() ==
          "test: value of attribute id=account_for_gravity of component id=42 does not have the correct value type");
  }
}

TEST_CASE("Component mapping test")
{
  const auto& dbModel = loadModel("1.4");

  SUBCASE("Get id")
  {
    rexsapi::detail::ComponentMapping mapping;
    auto component1Id = mapping.addComponent(42);
    auto component2Id = mapping.addComponent(43);
    auto component3Id = mapping.addComponent(44);

    rexsapi::TAttributes attributes;
    rexsapi::TComponents components;
    components.emplace_back(
      rexsapi::TComponent{component1Id, dbModel.findComponentById("gear_unit"), "Component 1", attributes});
    components.emplace_back(
      rexsapi::TComponent{component2Id, dbModel.findComponentById("gear_casing"), "Component 2", attributes});
    components.emplace_back(
      rexsapi::TComponent{component3Id, dbModel.findComponentById("material"), "Component 3", attributes});

    auto component = mapping.getComponent(42, components);
    REQUIRE(component);
    CHECK(component->getInternalId() == component1Id);
    component = mapping.getComponent(43, components);
    REQUIRE(component);
    CHECK(component->getInternalId() == component2Id);
    component = mapping.getComponent(44, components);
    REQUIRE(component);
    CHECK(component->getInternalId() == component3Id);

    CHECK_FALSE(mapping.getComponent(45, components));
  }
}

TEST_CASE("Component post processor test")
{
  const auto& dbModel = loadModel("1.4");
  rexsapi::TResult result;
  rexsapi::detail::TModeAdapter mode{rexsapi::TMode::STRICT_MODE};
  rexsapi::detail::ComponentMapping mapping;
  auto component1Id = mapping.addComponent(42);
  auto component2Id = mapping.addComponent(43);

  rexsapi::TComponents components;

  rexsapi::TAttributes attributes{
    rexsapi::TAttribute{dbModel.findAttributeById("temperature_lubricant"), rexsapi::TValue{73.2}},
    rexsapi::TAttribute{dbModel.findAttributeById("type_of_gear_casing_construction_vdi_2736_2014"),
                        rexsapi::TValue{"closed"}}};
  components.emplace_back(rexsapi::TComponent{component1Id, dbModel.findComponentById("gear_casing"), "", attributes});

  attributes.clear();
  attributes.emplace_back(
    rexsapi::TAttribute{dbModel.findAttributeById("density_at_15_degree_celsius"), rexsapi::TValue{1.02}});
  attributes.emplace_back(rexsapi::TAttribute{dbModel.findAttributeById("lubricant_type_iso_6336_2006"),
                                              rexsapi::TValue{"non_water_soluble_polyglycol"}});
  attributes.emplace_back(
    rexsapi::TAttribute{dbModel.findAttributeById("viscosity_at_100_degree_celsius"), rexsapi::TValue{37.0}});

  SUBCASE("Without reference components")
  {
    components.emplace_back(rexsapi::TComponent{component2Id, dbModel.findComponentById("lubricant"), "", attributes});
    rexsapi::detail::ComponentPostProcessor postProcessor{result, mode, components, mapping};
    auto processedComponents = postProcessor.release();

    CHECK(result);
    REQUIRE(processedComponents.size() == 2);
    CHECK(processedComponents[0].getAttributes().size() == 2);
    CHECK(processedComponents[1].getAttributes().size() == 3);
  }

  SUBCASE("With reference component")
  {
    attributes.emplace_back(
      rexsapi::TAttribute{dbModel.findAttributeById("reference_component_for_position"), rexsapi::TValue{42}});
    components.emplace_back(rexsapi::TComponent{component2Id, dbModel.findComponentById("lubricant"), "", attributes});
    rexsapi::detail::ComponentPostProcessor postProcessor{result, mode, components, mapping};
    auto processedComponents = postProcessor.release();

    CHECK(result);
    REQUIRE(processedComponents.size() == 2);
    CHECK(processedComponents[0].getAttributes().size() == 2);
    REQUIRE(processedComponents[1].getAttributes().size() == 4);
    CHECK(processedComponents[1].getAttributes()[3].getValue<rexsapi::TReferenceComponentType>() == component1Id);
  }

  SUBCASE("With non exisiting reference component")
  {
    attributes.emplace_back(
      rexsapi::TAttribute{dbModel.findAttributeById("reference_component_for_position"), rexsapi::TValue{815}});
    components.emplace_back(rexsapi::TComponent{component2Id, dbModel.findComponentById("lubricant"), "", attributes});
    rexsapi::detail::ComponentPostProcessor postProcessor{result, mode, components, mapping};
    auto processedComponents = postProcessor.release();

    CHECK_FALSE(result);
    REQUIRE(processedComponents.size() == 2);
    CHECK(processedComponents[0].getAttributes().size() == 2);
    CHECK(processedComponents[1].getAttributes().size() == 3);
  }
}
