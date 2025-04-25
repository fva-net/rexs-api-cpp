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

#include <rexsapi/ModelBuilder.hxx>

#include <test/TestHelper.hxx>
#include <test/TestModelLoader.hxx>

#include <doctest.h>


TEST_CASE("Component id test")
{
  SUBCASE("Int id")
  {
    rexsapi::TComponentId id{42};
    CHECK(id.asString() == "42");
    CHECK(id == rexsapi::TComponentId{42});
    CHECK_FALSE(id == rexsapi::TComponentId{43});
    CHECK(id < rexsapi::TComponentId{43});
    CHECK_FALSE(rexsapi::TComponentId{43} < id);
    CHECK_FALSE(id < rexsapi::TComponentId{42});
    CHECK(id.hash() == rexsapi::TComponentId{42}.hash());
    CHECK_FALSE(id.hash() == rexsapi::TComponentId{40}.hash());
  }

  SUBCASE("String id")
  {
    rexsapi::TComponentId id{"puschel"};
    CHECK(id.asString() == "puschel");
    CHECK(id == rexsapi::TComponentId{"puschel"});
    CHECK_FALSE(id == rexsapi::TComponentId{"hutzli"});
    CHECK(id < rexsapi::TComponentId{"putzli"});
    CHECK_FALSE(rexsapi::TComponentId{"putzli"} < id);
    CHECK_FALSE(id < rexsapi::TComponentId{"puschel"});
    CHECK(id.hash() == rexsapi::TComponentId{"puschel"}.hash());
    CHECK_FALSE(id.hash() == rexsapi::TComponentId{"putzli"}.hash());
  }

  SUBCASE("String and int id")
  {
    rexsapi::TComponentId nid{815};
    rexsapi::TComponentId sid{"815"};
    CHECK_FALSE(nid == sid);
    CHECK(nid < sid);
    CHECK_FALSE(sid < nid);
  }
}

TEST_CASE("Component builder test")
{
  const auto registry = createModelRegistry();
  rexsapi::TFileXsdSchemaLoader schemaLoader{projectDir() / "models" / "rexs-file.xsd"};
  rexsapi::TXSDSchemaValidator validator{schemaLoader};
  rexsapi::TComponentBuilder builder{registry.getModel({1, 5}, "de")};

  SUBCASE("Component builder")
  {
    builder.addComponent("cylindrical_gear").name("Zylinder").addAttribute("conversion_factor").value(2.11);
    auto gearId = builder.id();
    builder.addAttribute("display_color").value(rexsapi::TFloatArrayType{30.0, 10.0, 55.0}).unit("%");
    builder.addAttribute("support_vector").unit("mm").value(rexsapi::TFloatArrayType{70.0, 0.0, 0.0}).coded();
    builder.addAttribute("u_axis_vector")
      .unit("mm")
      .value(rexsapi::TFloatArrayType{0.0, 65.0, 0.0})
      .coded(rexsapi::TCodeType::Optimized);
    builder.addCustomAttribute("custom_hutzli", rexsapi::TValueType::BOOLEAN).value(true);
    auto casingId = builder.addComponent("gear_casing", "my-id")
                      .addAttribute("temperature_lubricant")
                      .value(128.0)
                      .addAttribute("modification_date")
                      .value(rexsapi::TDatetime{"2022-06-05T08:50:27+03:00"})
                      .id();
    CHECK(casingId == rexsapi::TComponentId{"my-id"});
    auto components = builder.build();
    REQUIRE(components.size() == 2);
    CHECK(components[0].getName() == "Zylinder");
    CHECK(components[0].getType() == "cylindrical_gear");
    CHECK(components[1].getName() == "");
    CHECK(components[1].getType() == "gear_casing");
    REQUIRE(components[0].getAttributes().size() == 5);
    CHECK(components[0].getAttributes()[0].getAttributeId() == "conversion_factor");
    CHECK(components[0].getAttributes()[0].getValueAsString() == "2.11");
    CHECK(components[0].getAttributes()[0].getValue().coded() == rexsapi::TCodeType::None);
    CHECK(components[0].getAttributes()[2].getAttributeId() == "support_vector");
    CHECK(components[0].getAttributes()[2].getValue().coded() == rexsapi::TCodeType::Default);
    CHECK(components[0].getAttributes()[3].getAttributeId() == "u_axis_vector");
    CHECK(components[0].getAttributes()[3].getValue().coded() == rexsapi::TCodeType::Optimized);
    CHECK(components[0].getAttributes()[4].isCustomAttribute());
    CHECK(components[0].getAttributes()[4].getUnit().getName().empty());
    CHECK(components[1].getInternalId() == builder.getComponentForId(components, casingId).getInternalId());
    CHECK(components[0].getInternalId() == builder.getComponentForId(components, gearId).getInternalId());
  }

  SUBCASE("Component builder errors")
  {
    CHECK_THROWS_WITH(builder.addAttribute("temperature_lubricant"), "no components added yet");
    CHECK_THROWS_WITH(builder.name("puschel"), "no components added yet");
    CHECK_THROWS_WITH(builder.addComponent("cylindrical_gear").value(47.11), "no attributes added yet");
    CHECK_THROWS_WITH(builder.unit("%"), "no attributes added yet");
  }
}

TEST_CASE("Model builder test")
{
  const auto registry = createModelRegistry();
  rexsapi::TFileXsdSchemaLoader schemaLoader{projectDir() / "models" / "rexs-file.xsd"};
  rexsapi::TXSDSchemaValidator validator{schemaLoader};
  rexsapi::TModelBuilder builder{registry.getModel({1, 4}, "de")};

  SUBCASE("Model builder no load cases")
  {
    auto id =
      builder.addComponent("cylindrical_gear").name("Zylinder").addAttribute("conversion_factor").value(2.11).id();
    builder.addAttribute("display_color").value(rexsapi::TFloatArrayType{30.0, 10.0, 55.0}).unit("%");
    builder.addCustomAttribute("custom_puschel", rexsapi::TValueType::STRING).unit("none").value("hutzli");
    builder.addComponent("gear_casing", "my-id").addAttribute("temperature_lubricant").value(128.0);
    builder.addRelation(rexsapi::TRelationType::ASSEMBLY).addRef(rexsapi::TRelationRole::GEAR, id);
    builder.addRef(rexsapi::TRelationRole::PART, "my-id").order(1);
    builder.hint("some hint");
    auto model = builder.build("Test Appl", "1.35", "en");
    CHECK(model.getInfo().getApplicationId() == "Test Appl");
    CHECK(model.getInfo().getApplicationVersion() == "1.35");
    CHECK(*model.getInfo().getApplicationLanguage() == "en");
    CHECK(model.getInfo().getVersion() == rexsapi::TRexsVersion{1, 4});
    REQUIRE(model.getComponents().size() == 2);
    REQUIRE(model.getComponents()[0].getAttributes().size() == 3);
    CHECK(model.getComponents()[0].getAttributes()[0].getAttributeId() == "conversion_factor");
    CHECK(model.getComponents()[0].getAttributes()[1].getAttributeId() == "display_color");
    CHECK(model.getComponents()[0].getAttributes()[2].getAttributeId() == "custom_puschel");
    REQUIRE(model.getRelations().size() == 1);
    CHECK(model.getRelations()[0].getType() == rexsapi::TRelationType::ASSEMBLY);
    CHECK(model.getRelations()[0].getOrder().has_value());
    CHECK_FALSE(model.getLoadSpectrum().hasLoadCases());
    CHECK_FALSE(model.getLoadSpectrum().hasAccumulation());
  }

  SUBCASE("Model builder with load cases")
  {
    auto id =
      builder.addComponent("cylindrical_gear").name("Zylinder").addAttribute("conversion_factor").value(2.11).id();
    builder.addAttribute("display_color").value(rexsapi::TFloatArrayType{30.0, 10.0, 55.0}).unit("%");
    builder.addCustomAttribute("custom_puschel", rexsapi::TValueType::STRING).unit("m").value("hutzli");
    builder.addComponent("gear_casing", "my-id").addAttribute("temperature_lubricant").value(128.0);
    builder.addRelation(rexsapi::TRelationType::ASSEMBLY).addRef(rexsapi::TRelationRole::GEAR, id);
    builder.addRef(rexsapi::TRelationRole::PART, "my-id").order(1);
    builder.hint("some hint");
    builder.addLoadCase().addComponent(id).addAttribute("torque_acting_on_shaft_u").value(4.77);
    builder.addLoadCase().addComponent("my-id").addAttribute("operating_viscosity").value(0.11);
    auto model = builder.build("Test Appl", "1.35", {});
    CHECK(model.getInfo().getApplicationId() == "Test Appl");
    CHECK(model.getInfo().getApplicationVersion() == "1.35");
    CHECK_FALSE(model.getInfo().getApplicationLanguage());
    CHECK(model.getInfo().getVersion() == rexsapi::TRexsVersion{1, 4});
    REQUIRE(model.getComponents().size() == 2);
    REQUIRE(model.getComponents()[0].getAttributes().size() == 3);
    CHECK(model.getComponents()[0].getAttributes()[0].getAttributeId() == "conversion_factor");
    CHECK(model.getComponents()[0].getAttributes()[1].getAttributeId() == "display_color");
    CHECK(model.getComponents()[0].getAttributes()[2].getAttributeId() == "custom_puschel");
    REQUIRE(model.getRelations().size() == 1);
    CHECK(model.getRelations()[0].getType() == rexsapi::TRelationType::ASSEMBLY);
    CHECK(model.getRelations()[0].getOrder().has_value());
    CHECK(model.getLoadSpectrum().hasLoadCases());
    CHECK_FALSE(model.getLoadSpectrum().hasAccumulation());
  }

  SUBCASE("Model builder with load cases & accumulation")
  {
    auto id =
      builder.addComponent("cylindrical_gear").name("Zylinder").addAttribute("conversion_factor").value(2.11).id();
    builder.addAttribute("display_color").value(rexsapi::TFloatArrayType{30.0, 10.0, 55.0}).unit("%");
    builder.addCustomAttribute("custom_puschel", rexsapi::TValueType::STRING).unit("m").value("hutzli");
    builder.addComponent("gear_casing", "my-id").addAttribute("temperature_lubricant").value(128.0);
    builder.addRelation(rexsapi::TRelationType::ASSEMBLY).addRef(rexsapi::TRelationRole::GEAR, id);
    builder.addRef(rexsapi::TRelationRole::PART, "my-id").order(1);
    builder.hint("some hint");
    builder.addLoadCase().addComponent(id).addAttribute("torque_acting_on_shaft_u").value(4.77);
    builder.addLoadCase().addComponent("my-id").addAttribute("operating_viscosity").value(0.11);
    builder.addAccumulation().addComponent(id).addAttribute("bulk_temperature").value(37.5);
    auto model = builder.build("Test Appl", "1.35", {});
    CHECK(model.getInfo().getApplicationId() == "Test Appl");
    CHECK(model.getInfo().getApplicationVersion() == "1.35");
    CHECK_FALSE(model.getInfo().getApplicationLanguage());
    CHECK(model.getInfo().getVersion() == rexsapi::TRexsVersion{1, 4});
    REQUIRE(model.getComponents().size() == 2);
    REQUIRE(model.getComponents()[0].getAttributes().size() == 3);
    CHECK(model.getComponents()[0].getAttributes()[0].getAttributeId() == "conversion_factor");
    CHECK(model.getComponents()[0].getAttributes()[1].getAttributeId() == "display_color");
    CHECK(model.getComponents()[0].getAttributes()[2].getAttributeId() == "custom_puschel");
    REQUIRE(model.getRelations().size() == 1);
    CHECK(model.getRelations()[0].getType() == rexsapi::TRelationType::ASSEMBLY);
    CHECK(model.getRelations()[0].getOrder().has_value());
    CHECK(model.getLoadSpectrum().hasLoadCases());
    CHECK(model.getLoadSpectrum().hasAccumulation());
  }

  SUBCASE("Model builder errors")
  {
    CHECK_THROWS(builder.addAttribute("display_color"));
    CHECK_THROWS(builder.name("Puschel"));
    auto casingId = builder.addComponent("cylindrical_gear").id();
    auto gearId = builder.addComponent("gear_casing").id();
    CHECK_THROWS(builder.value(47.11));
    CHECK_THROWS(builder.unit("%"));
    CHECK_THROWS_WITH(
      builder.addAttribute("temperature_lubricant").value("puschel"),
      "value of attribute id=temperature_lubricant of component id=2 does not have the correct value type");
    builder.value(128.0);
    builder.addComponent("gear_casing", "da casing");
    CHECK_THROWS_WITH(builder.addComponent("gear_casing", "da casing"), "component id=da casing already added");
    builder.addAttribute("operating_viscosity").value(0.11);
    CHECK_THROWS_WITH(builder.unit("kg"), "unit kg does not match attribute id=operating_viscosity unit");
    CHECK_THROWS_WITH(builder.addAttribute("operating_viscosity"),
                      "attribute id=operating_viscosity already added to component id=da casing");

    CHECK_THROWS_WITH(builder.order(5), "no relations added yet");
    CHECK_THROWS_WITH(builder.addRef(rexsapi::TRelationRole::PART, "4711"), "no relations added yet");
    builder.addRelation(rexsapi::TRelationType::ASSEMBLY);
    CHECK_THROWS_WITH(builder.hint("hint"), "no references added yet");
    builder.addRef(rexsapi::TRelationRole::GEAR, gearId);
    builder.addRef(rexsapi::TRelationRole::PART, "my-id");
    CHECK_THROWS_WITH((void)builder.build("Test Appl", "1.35", {}), "no component found for id=my-id");
  }

  SUBCASE("Model builder creation errors")
  {
    CHECK_THROWS_WITH((void)builder.build("Test Appl", "1.35", {}), "no components specified for model");
    auto gearId = builder.addComponent("cylindrical_gear").id();
    builder.addAttribute("torque_acting_on_shaft_u").value(4.77);
    // TODO: remove
    // CHECK_THROWS_WITH((void)builder.build("Test Appl", "1.35", {}), "no relations specified for model");
    builder.addRelation(rexsapi::TRelationType::ASSEMBLY);
    CHECK_THROWS_WITH((void)builder.build("Test Appl", "1.35", {}), "no references specified for relation");
    builder.addRef(rexsapi::TRelationRole::GEAR, gearId);
    builder.addComponent("bevel_stage", "stage").name("Stufe").addAttribute("hypoid_offset").value(2.11);
    CHECK_THROWS_WITH((void)builder.build("Test Appl", "1.35", {}), "1 components are not used in a relation");
    builder.addRef(rexsapi::TRelationRole::STAGE, "stage");
    builder.addAttribute("deviation_of_offset");
    CHECK_THROWS_WITH((void)builder.build("Test Appl", "1.35", {}),
                      "attribute id=deviation_of_offset has an empty value");
    builder.value(47.11);
    auto& loadCase = builder.addLoadCase();
    loadCase.addComponent(gearId);
    loadCase.addAttribute("blade_point_radius");
    CHECK_THROWS_WITH((void)builder.build("Test Appl", "1.35", {}),
                      "attribute id=blade_point_radius is not part of component cylindrical_gear id=1");
    builder.addAttribute("blade_profile_height").value(0.815);
    CHECK_THROWS_WITH((void)builder.build("Test Appl", "1.35", {}),
                      "attribute id=blade_profile_height is not part of component bevel_stage id=stage");
  }
}
