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

  SUBCASE("Merge")
  {
    std::optional<rexsapi::TModel> newModel;

    {
      const auto mainModel =
        loader.load(projectDir() / "test" / "example_models" / "external_sources" / "placeholder_model.rexs", result,
                    rexsapi::TMode::RELAXED_MODE);
      const auto referencedModel1 =
        loader.load(projectDir() / "test" / "example_models" / "external_sources" / "database_shaft.rexs", result,
                    rexsapi::TMode::RELAXED_MODE);
      const auto referencedModel2 =
        loader.load(projectDir() / "test" / "example_models" / "external_sources" / "database_bearing.rexs", result,
                    rexsapi::TMode::RELAXED_MODE);

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
    CHECK(newModel->getRelations().size() == 8);
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
}
