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
#include <test/TestModelLoader.hxx>

#include <doctest.h>


TEST_CASE("Model merger test")
{
  const rexsapi::TModelLoader loader{projectDir() / "models"};
  const auto registry = createModelRegistry();
  rexsapi::TResult result;
  rexsapi::TModelMerger merger{registry};

  SUBCASE("Merge")
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

    auto newModel = merger.merge(result, *mainModel, "./database_shaft.rexs", *referencedModel1);
    newModel = merger.merge(result, *newModel, "./database_bearing.rexs", *referencedModel2);

    for (const auto& comp : newModel->getComponents()) {
      std::cout << comp.getType() << "\n";
      for (const auto& att : comp.getAttributes()) {
        std::cout << "\t" << att.getAttributeId() << "\n";
      }
    }
    CHECK(newModel);
    CHECK(result);
  }

  SUBCASE("Merge with different rexs versions")
  {
    rexsapi::TComponents components;
    rexsapi::TRelations relations;
    rexsapi::TLoadCases loadCases;
    rexsapi::TLoadSpectrum spectrum{loadCases, {}};

    rexsapi::TModelInfo modelInfo15{"My App", "", "2024-03-13", rexsapi::TRexsVersion{"1.5"}, {}};
    rexsapi::TModel mainModel{modelInfo15, components, relations, spectrum};

    rexsapi::TModelInfo modelInfo16{"My App", "", "2024-03-13", rexsapi::TRexsVersion{"1.6"}, {}};
    rexsapi::TModel referencedModel{modelInfo16, components, relations, spectrum};

    auto newModel = merger.merge(result, mainModel, "", referencedModel);
    CHECK_FALSE(newModel);
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 1);
    CHECK(result.getErrors()[0].getMessage() == "cannot reference components from different rexs versions");
    CHECK(result.getErrors()[0].isError());
  }
}
