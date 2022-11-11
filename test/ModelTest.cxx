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

#include <rexsapi/Model.hxx>

#include <test/TestModelLoader.hxx>

#include <doctest.h>

TEST_CASE("Model test")
{
  const auto& dbModel = loadModel("1.4");

  SUBCASE("Create")
  {
    const auto* dbComponent = &dbModel.findComponentById("gear_casing");

    rexsapi::TAttributes attributes{
      rexsapi::TAttribute{dbComponent->findAttributeById("temperature_lubricant"), rexsapi::TValue{73.2}},
      rexsapi::TAttribute{dbComponent->findAttributeById("type_of_gear_casing_construction_vdi_2736_2014"),
                          rexsapi::TValue{"closed"}}};

    rexsapi::TComponents components;
    components.emplace_back(rexsapi::TComponent{1, *dbComponent, "Gehäuse", std::move(attributes)});

    dbComponent = &dbModel.findComponentById("lubricant");
    attributes = rexsapi::TAttributes{};
    attributes.emplace_back(
      rexsapi::TAttribute{dbComponent->findAttributeById("density_at_15_degree_celsius"), rexsapi::TValue{1.02}});
    attributes.emplace_back(rexsapi::TAttribute{dbComponent->findAttributeById("lubricant_type_iso_6336_2006"),
                                                rexsapi::TValue{"non_water_soluble_polyglycol"}});
    attributes.emplace_back(rexsapi::TAttribute{dbComponent->findAttributeById("name"), rexsapi::TValue{"PG"}});
    attributes.emplace_back(
      rexsapi::TAttribute{dbComponent->findAttributeById("viscosity_at_100_degree_celsius"), rexsapi::TValue{37.0}});
    attributes.emplace_back(
      rexsapi::TAttribute{dbComponent->findAttributeById("viscosity_at_40_degree_celsius"), rexsapi::TValue{220.0}});

    components.emplace_back(rexsapi::TComponent{2, *dbComponent, "S2/220", std::move(attributes)});

    rexsapi::TRelationReferences references{
      rexsapi::TRelationReference{rexsapi::TRelationRole::ORIGIN, "hint0", components[0]},
      rexsapi::TRelationReference{rexsapi::TRelationRole::REFERENCED, "hint1", components[1]}};

    rexsapi::TRelations relations{rexsapi::TRelation{rexsapi::TRelationType::REFERENCE, {}, std::move(references)}};

    rexsapi::TModelInfo info{"FVA Workbench", "7.1 - DEV gültig bis 30.4.2022", "2021-12-14T15:56:10+01:00",
                             rexsapi::TRexsVersion{"1.4"}, "de"};
    rexsapi::TLoadSpectrum spectrum{rexsapi::TLoadCases{}, {}};
    rexsapi::TModel model{info, std::move(components), std::move(relations), std::move(spectrum)};

    CHECK(model.getInfo().getApplicationId() == "FVA Workbench");
    CHECK(model.getInfo().getVersion() == rexsapi::TRexsVersion{"1.4"});
    CHECK(*model.getInfo().getApplicationLanguage() == "de");
    REQUIRE(model.getComponents().size() == 2);
    CHECK(model.getComponents()[0].getType() == "gear_casing");
    REQUIRE(model.getRelations().size() == 1);
    REQUIRE(model.getRelations()[0].getReferences().size() == 2);
    CHECK(model.getRelations()[0].getReferences()[0].getComponent().getType() == "gear_casing");
    CHECK(model.getRelations()[0].getReferences()[0].getHint() == "hint0");
    CHECK(model.getRelations()[0].getReferences()[0].getRole() == rexsapi::TRelationRole::ORIGIN);
    CHECK(model.getRelations()[0].getReferences()[1].getComponent().getType() == "lubricant");
    REQUIRE(model.getRelations()[0].getReferences()[1].getComponent().getAttributes().size() == 5);
    const auto& atts = model.getRelations()[0].getReferences()[1].getComponent().getAttributes();
    CHECK(atts[0].getAttributeId() == "density_at_15_degree_celsius");
    CHECK(atts[0].getValueType() == rexsapi::TValueType::FLOATING_POINT);
    CHECK(atts[0].getValue<double>() == doctest::Approx{1.02});
  }
}
