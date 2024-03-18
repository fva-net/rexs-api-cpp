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

#include <rexsapi/ModelLoader.hxx>
#include <rexsapi/ModelMerger.hxx>

#include <test/TestHelper.hxx>
#include <test/TestModelHelper.hxx>
#include <test/TestModelLoader.hxx>

#include <doctest.h>


TEST_CASE("Model merger test")
{
  const rexsapi::TModelLoader loader{projectDir() / "models"};
  const auto registry = createModelRegistry();
  rexsapi::TResult result;
  const rexsapi::TModelMerger merger{registry};

  SUBCASE("Merge multiple models")
  {
    std::optional<rexsapi::TModel> newModel;

    {
      const auto mainModel = loader.load(projectDir() / "test" / "example_models" / "external_sources" / "example_1" /
                                           "placeholder_model.rexs",
                                         result, rexsapi::TMode::RELAXED_MODE);
      const auto referencedModel1 =
        loader.load(projectDir() / "test" / "example_models" / "external_sources" / "example_1" / "database_shaft.rexs",
                    result, rexsapi::TMode::RELAXED_MODE);
      const auto referencedModel2 = loader.load(projectDir() / "test" / "example_models" / "external_sources" /
                                                  "example_1" / "database_bearing.rexs",
                                                result, rexsapi::TMode::RELAXED_MODE);

      newModel = merger.merge(result, *mainModel, "./database_shaft.rexs", *referencedModel1);
      newModel = merger.merge(result, *newModel, "./database_bearing.rexs", *referencedModel2);
    }

    CHECK(newModel);
    CHECK(result);
    CHECK(newModel->getComponents().size() == 9);
    const ComponentFinder finder{*newModel};
    CHECK_NOTHROW(finder.findComponent("18CrMo4 [50]"));
    CHECK_NOTHROW(finder.findComponent("section [100]"));
    REQUIRE_NOTHROW(finder.findComponent("section [110]"));
    CHECK_NOTHROW(finder.findComponent("rolling_bearing_row [80]"));
    CHECK_NOTHROW(finder.findComponent("rolling_bearing_row [90]"));
    REQUIRE_NOTHROW(finder.findComponent("Rolling bearing [6]"));
    const auto& bearing = finder.findComponent("Rolling bearing [6]");
    CHECK(bearing.getAttributes().size() == 33);
    CHECK(finder.findComponentsByType("shaft_section").size() == 2);
    // TODO: fix
    // CHECK(newModel->getRelations().size() == 8);
  }

  SUBCASE("Merge with different rexs versions")
  {
    const rexsapi::TModelInfo modelInfo15{"My App", "", "2024-03-13", rexsapi::TRexsVersion{"1.5"}, {}};
    const rexsapi::TModel mainModel{modelInfo15, rexsapi::TComponents{}, rexsapi::TRelations{},
                                    rexsapi::TLoadSpectrum{rexsapi::TLoadCases{}, {}}};

    const rexsapi::TModelInfo modelInfo16{"My App", "", "2024-03-13", rexsapi::TRexsVersion{"1.6"}, {}};
    const rexsapi::TModel referencedModel{modelInfo16, rexsapi::TComponents{}, rexsapi::TRelations{},
                                          rexsapi::TLoadSpectrum{rexsapi::TLoadCases{}, {}}};

    const auto newModel = merger.merge(result, mainModel, "", referencedModel);
    CHECK_FALSE(newModel);
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 1);
    CHECK(result.getErrors()[0].getMessage() == "cannot reference components from different rexs versions");
    CHECK(result.getErrors()[0].isError());
  }

  SUBCASE("Merge model with components only")
  {
    std::optional<rexsapi::TModel> newModel;

    {
      const auto mainModel = loader.load(projectDir() / "test" / "example_models" / "external_sources" / "example_2" /
                                           "placeholder_model.rexs",
                                         result, rexsapi::TMode::RELAXED_MODE);
      const auto referencedModel = loader.load(projectDir() / "test" / "example_models" / "external_sources" /
                                                 "example_2" / "database_material.rexs",
                                               result, rexsapi::TMode::RELAXED_MODE);

      newModel = merger.merge(result, *mainModel, "./database_material.rexs", *referencedModel);
    }

    CHECK(newModel);
    CHECK(result);
    CHECK(newModel->getComponents().size() == 7);

    const ComponentFinder finder{*newModel};
    REQUIRE_NOTHROW(finder.findComponent("18CrMo4 [5]"));
    CHECK(finder.findComponent("18CrMo4 [5]").getType() == "material");
    CHECK(finder.findComponent("18CrMo4 [5]").getAttributes().size() == 5);
    REQUIRE_NOTHROW(finder.findComponent("18CrMo4 [6]"));
    CHECK(finder.findComponent("18CrMo4 [6]").getType() == "material");
    CHECK(finder.findComponent("18CrMo4 [6]").getAttributes().size() == 5);
    REQUIRE_NOTHROW(finder.findComponent("17CrNi6-6 [7]"));
    CHECK(finder.findComponent("17CrNi6-6 [7]").getType() == "material");
    const auto& attributes = finder.findComponent("17CrNi6-6 [7]").getAttributes();
    REQUIRE(attributes.size() == 5);
    CHECK(attributes[2].getAttributeId() == "fatigue_limit_bending");
    CHECK(attributes[2].getValue<rexsapi::TFloatType>() == doctest::Approx(300.0));
    CHECK(attributes[3].getAttributeId() == "fatigue_limit_compression_tension");
    CHECK(attributes[3].getValue<rexsapi::TFloatType>() == doctest::Approx(240.0));
    CHECK(attributes[4].getAttributeId() == "fatigue_limit_torsion");
    CHECK(attributes[4].getValue<rexsapi::TFloatType>() == doctest::Approx(360.0));
  }

  SUBCASE("Merge model with non existing component in data_source")
  {
    std::optional<rexsapi::TModel> newModel;

    {
      const auto mainModel = loader.load(projectDir() / "test" / "example_models" / "external_sources" / "example_3" /
                                           "placeholder_model.rexs",
                                         result, rexsapi::TMode::RELAXED_MODE);
      const auto referencedModel = loader.load(projectDir() / "test" / "example_models" / "external_sources" /
                                                 "example_3" / "database_material.rexs",
                                               result, rexsapi::TMode::RELAXED_MODE);

      result.reset();
      newModel = merger.merge(result, *mainModel, "./database_material.rexs", *referencedModel);
    }

    CHECK_FALSE(newModel);
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 1);
    CHECK(result.getErrors()[0].getMessage() ==
          "cannot find referenced component 42 in data_source './database_material.rexs'");
    CHECK(result.getErrors()[0].isError());
  }

  SUBCASE("Merge model with wrong component type in data_source")
  {
    std::optional<rexsapi::TModel> newModel;

    {
      const auto mainModel = loader.load(projectDir() / "test" / "example_models" / "external_sources" / "example_4" /
                                           "placeholder_model.rexs",
                                         result, rexsapi::TMode::RELAXED_MODE);
      const auto referencedModel = loader.load(projectDir() / "test" / "example_models" / "external_sources" /
                                                 "example_4" / "database_material.rexs",
                                               result, rexsapi::TMode::RELAXED_MODE);

      result.reset();
      newModel = merger.merge(result, *mainModel, "./database_material.rexs", *referencedModel);
    }

    CHECK_FALSE(newModel);
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 1);
    CHECK(
      result.getErrors()[0].getMessage() ==
      "referenced component 8 in data_source './database_material.rexs' has wrong type 'material' instead of 'shaft'");
    CHECK(result.getErrors()[0].isError());
  }
}
