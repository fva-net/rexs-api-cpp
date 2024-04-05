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

#include <rexsapi/ExternalSubcomponentsChecker.hxx>

#include <test/TestModelLoader.hxx>

#include <doctest.h>

TEST_CASE("External permissible subcomponent checker test")
{
  const auto& dbModel = loadModel("1.5");
  rexsapi::TAttributes attributes{};

  rexsapi::TResult result;

  SUBCASE("load unkown version")
  {
    auto mainComponent = rexsapi::TComponent{42, dbModel.findComponentById("material"), "", attributes};
    auto subComponent = rexsapi::TComponent{43, dbModel.findComponentById("material"), "", attributes};

    rexsapi::TExternalSubcomponentsChecker checker{rexsapi::TMode::STRICT_MODE, rexsapi::TRexsVersion{1, 0}};
    CHECK_FALSE(checker.isPermissibleSubComponent(result, mainComponent, subComponent));
    CHECK_FALSE(result);
  }

  SUBCASE("find permissible subcomponents")
  {
    auto mainComponent =
      rexsapi::TComponent{42, dbModel.findComponentById("rolling_bearing_with_catalog_geometry"), "", attributes};
    auto subComponent = rexsapi::TComponent{43, dbModel.findComponentById("rolling_bearing_row"), "", attributes};

    rexsapi::TExternalSubcomponentsChecker checker{rexsapi::TMode::STRICT_MODE, rexsapi::TRexsVersion{1, 5}};
    CHECK(checker.isPermissibleSubComponent(result, mainComponent, subComponent));
    CHECK(result);

    mainComponent = rexsapi::TComponent{42, dbModel.findComponentById("ring_gear"), "", attributes};
    subComponent = rexsapi::TComponent{43, dbModel.findComponentById("end_relief_non_datum_face"), "", attributes};
    CHECK(checker.isPermissibleSubComponent(result, mainComponent, subComponent));
    CHECK(result);
  }

  SUBCASE("find non permissible subcomponents")
  {
    auto mainComponent = rexsapi::TComponent{42, dbModel.findComponentById("shaft"), "", attributes};
    auto subComponent = rexsapi::TComponent{43, dbModel.findComponentById("rolling_bearing_row"), "", attributes};

    rexsapi::TExternalSubcomponentsChecker checker{rexsapi::TMode::STRICT_MODE, rexsapi::TRexsVersion{1, 5}};
    CHECK_FALSE(checker.isPermissibleSubComponent(result, mainComponent, subComponent));
    CHECK_FALSE(result);
    CHECK(result.getErrors()[0].isError());
    CHECK(result.getErrors()[0].getMessage() ==
          "external sub component 'rolling_bearing_row' is not permissible for main component 'shaft'");
  }
}
