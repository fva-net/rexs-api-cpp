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

#include <rexsapi/LoadSpectrum.hxx>

#include <test/TestModelLoader.hxx>

#include <doctest.h>

TEST_CASE("Load spectrum test")
{
  const auto& dbModel = loadModel("1.4");

  rexsapi::TComponents components;
  const auto& gearCasingComponent = dbModel.findComponentById("gear_casing");

  rexsapi::TAttributes attributes;
  attributes.emplace_back(
    rexsapi::TAttribute{gearCasingComponent.findAttributeById("temperature_lubricant"), rexsapi::TValue{73.2}});
  attributes.emplace_back(
    rexsapi::TAttribute{gearCasingComponent.findAttributeById("type_of_gear_casing_construction_vdi_2736_2014"),
                        rexsapi::TValue{"closed"}});
  components.emplace_back(rexsapi::TComponent{1, gearCasingComponent, "Gehäuse", std::move(attributes)});

  const auto& lubricantComponent = dbModel.findComponentById("lubricant");
  attributes = rexsapi::TAttributes{};
  attributes.emplace_back(
    rexsapi::TAttribute{lubricantComponent.findAttributeById("density_at_15_degree_celsius"), rexsapi::TValue{1.02}});
  attributes.emplace_back(rexsapi::TAttribute{lubricantComponent.findAttributeById("lubricant_type_iso_6336_2006"),
                                              rexsapi::TValue{"non_water_soluble_polyglycol"}});
  components.emplace_back(rexsapi::TComponent{2, lubricantComponent, "S2/220", std::move(attributes)});

  SUBCASE("Load case")
  {
    rexsapi::TAttributes loadAttributes;
    loadAttributes.emplace_back(
      rexsapi::TAttribute{gearCasingComponent.findAttributeById("mass_of_component"), rexsapi::TValue{10.5}});
    loadAttributes.emplace_back(
      rexsapi::TAttribute{gearCasingComponent.findAttributeById("mean_operating_temperature"), rexsapi::TValue{55.5}});

    rexsapi::TLoadComponents loadComponents;
    loadComponents.emplace_back(rexsapi::TLoadComponent{components[0], std::move(loadAttributes)});

    loadAttributes = rexsapi::TAttributes{};
    loadAttributes.emplace_back(rexsapi::TAttribute{
      lubricantComponent.findAttributeById("viscosity_at_100_degree_celsius"), rexsapi::TValue{5.5}});
    loadComponents.emplace_back(rexsapi::TLoadComponent{components[1], std::move(loadAttributes)});

    rexsapi::TLoadCase loadCase{std::move(loadComponents)};

    REQUIRE(loadCase.getLoadComponents().size() == 2);
    CHECK(loadCase.getLoadComponents()[0].getComponent().getType() == "gear_casing");
    CHECK(loadCase.getLoadComponents()[0].getComponent().getAttributes().size() == 2);
    CHECK(loadCase.getLoadComponents()[0].getAttributes().size() == 4);
    CHECK(loadCase.getLoadComponents()[0].getLoadAttributes().size() == 2);
    CHECK(loadCase.getLoadComponents()[1].getComponent().getType() == "lubricant");
    CHECK(loadCase.getLoadComponents()[1].getComponent().getAttributes().size() == 2);
    CHECK(loadCase.getLoadComponents()[1].getLoadAttributes().size() == 1);

    rexsapi::TLoadComponents accumulationComponents;
    loadAttributes = rexsapi::TAttributes{};
    loadAttributes.emplace_back(
      rexsapi::TAttribute{gearCasingComponent.findAttributeById("operating_viscosity"), rexsapi::TValue{15.5}});
    loadAttributes.emplace_back(
      rexsapi::TAttribute{gearCasingComponent.findAttributeById("mean_operating_temperature"), rexsapi::TValue{60.5}});
    accumulationComponents.emplace_back(rexsapi::TLoadComponent{components[0], std::move(loadAttributes)});

    rexsapi::TAccumulation accumulation{std::move(accumulationComponents)};

    rexsapi::TLoadCases loadCases{std::move(loadCase)};
    rexsapi::TLoadSpectrum loadSpectrum{std::move(loadCases), std::move(accumulation)};

    CHECK(loadSpectrum.hasLoadCases());
    REQUIRE(loadSpectrum.getLoadCases().size() == 1);
    CHECK(loadSpectrum.getLoadCases()[0].getLoadComponents().size() == 2);
    REQUIRE(loadSpectrum.hasAccumulation());
    REQUIRE(loadSpectrum.getAccumulation().getLoadComponents().size() == 1);
    CHECK(loadSpectrum.getAccumulation().getLoadComponents()[0].getLoadAttributes().size() == 2);
  }
}
