
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

#include <rexsapi/DataSourceLoader.hxx>
#include <rexsapi/ModelLoader.hxx>

#include <test/TestHelper.hxx>
#include <test/TestModelHelper.hxx>

#include <doctest.h>

TEST_CASE("Data source loader tests")
{
  rexsapi::TResult result;

  SUBCASE("Load model with references")
  {
    const rexsapi::TDataSourceLoader dataSourceLoader{
      projectDir() / "models", projectDir() / "test" / "example_models" / "external_sources" / "example_1"};
    const rexsapi::TModelLoader modelLoader{projectDir() / "models", &dataSourceLoader};

    const auto model = modelLoader.load(projectDir() / "test" / "example_models" / "external_sources" / "example_1" /
                                          "placeholder_model.rexs",
                                        result, rexsapi::TMode::STRICT_MODE);
    CHECK(model);
    CHECK(result);
    CHECK(model->getComponents().size() == 9);
    const ComponentFinder finder{*model};
    CHECK_NOTHROW(finder.findComponent("18CrMo4 [50]"));
    CHECK_NOTHROW(finder.findComponent("section [100]"));
    REQUIRE_NOTHROW(finder.findComponent("section [110]"));
    CHECK_NOTHROW(finder.findComponent("rolling_bearing_row [80]"));
    CHECK_NOTHROW(finder.findComponent("rolling_bearing_row [90]"));
    REQUIRE_NOTHROW(finder.findComponent("Rolling bearing [6]"));
    const auto& bearing = finder.findComponent("Rolling bearing [6]");
    CHECK(bearing.getAttributes().size() == 31);
    CHECK(finder.findComponentsByType("shaft_section").size() == 2);
    CHECK(model->getRelations().size() == 8);
  }

  SUBCASE("Load non-existent referenced model")
  {
    const rexsapi::TDataSourceLoader dataSourceLoader{
      projectDir() / "models", projectDir() / "test" / "example_models" / "external_sources" / "example_7"};
    const rexsapi::TModelLoader modelLoader{projectDir() / "models", &dataSourceLoader};

    const auto model = modelLoader.load(projectDir() / "test" / "example_models" / "external_sources" / "example_7" /
                                          "placeholder_model.rexs",
                                        result, rexsapi::TMode::STRICT_MODE);

    CHECK_FALSE(model);
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 2);
    CHECK(result.getErrors()[1].getMessage() == "./database_bearing.rexs: could not load external referenced model");
  }

  SUBCASE("No data resolver")
  {
    const rexsapi::TModelLoader modelLoader{projectDir() / "models"};

    const auto model = modelLoader.load(projectDir() / "test" / "example_models" / "external_sources" / "example_7" /
                                          "placeholder_model.rexs",
                                        result, rexsapi::TMode::STRICT_MODE);

    CHECK(model);
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 2);
    CHECK(result.getErrors()[0].getMessage() ==
          "model contains external referenced components but no data source resolver was given");
    CHECK(result.getErrors()[1].getMessage() == "could not resolve all external referenced components");
  }

  SUBCASE("Could not resolve all references")
  {
    const rexsapi::TDataSourceLoader dataSourceLoader{
      projectDir() / "models", projectDir() / "test" / "example_models" / "external_sources" / "example_9"};
    const rexsapi::TModelLoader modelLoader{projectDir() / "models", &dataSourceLoader};

    const auto model = modelLoader.load(projectDir() / "test" / "example_models" / "external_sources" / "example_9" /
                                          "placeholder_model.rexs",
                                        result, rexsapi::TMode::STRICT_MODE);
    CHECK_FALSE(model);
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 2);
    CHECK(result.getErrors()[0].getMessage() ==
          "cannot find referenced component 60 in data_source './database_shaft.rexs'");
    CHECK(result.getErrors()[1].getMessage() ==
          "could not merge external referenced model from './database_shaft.rexs'");
  }
}
