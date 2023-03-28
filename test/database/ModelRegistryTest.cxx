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

#include <rexsapi/Rexsapi.hxx>

#include <test/TestHelper.hxx>

#include <doctest.h>

TEST_CASE("Test rexs model registry")
{
  const rexsapi::TFileXsdSchemaLoader schemaLoader{projectDir() / "models" / "rexs-dbmodel.xsd"};
  const rexsapi::database::TFileResourceLoader resourceLoader{projectDir() / "models"};
  const rexsapi::database::TXmlModelLoader modelLoader{resourceLoader, schemaLoader};
  const auto [registry, success] = rexsapi::database::TModelRegistry::createModelRegistry(modelLoader);
  REQUIRE(success);

  SUBCASE("Get existing models")
  {
    const auto& model = registry.getModel(rexsapi::TRexsVersion{"1.4"}, "de");
    CHECK(model.getVersion() == rexsapi::TRexsVersion{"1.4"});
    CHECK(model.getLanguage() == "de");

    const auto& component = model.findComponentById("side_plate");
    CHECK(component.getComponentId() == "side_plate");
    CHECK(component.getName() == "Wange");
    CHECK(component.getAttributes().size() == 9);

    CHECK(model.findUnitByName("mm").getId() == 2);
    CHECK(model.findUnitById(2).getName() == "mm");
  }

  SUBCASE("Get non existing models")
  {
    CHECK_THROWS_WITH((void)registry.getModel(rexsapi::TRexsVersion{"1.4"}, "es"),
                      "cannot find a database model for version '1.4' and locale 'es'");
    CHECK_THROWS_WITH((void)registry.getModel(rexsapi::TRexsVersion{"1.99"}, "es"),
                      "cannot find a database model for version '1.99' and locale 'es'");
  }

  SUBCASE("Get non existing model non-strict mode")
  {
    const auto& model = registry.getModel(rexsapi::TRexsVersion{"1.99"}, "de", false);
    CHECK(model.getVersion() == rexsapi::TRexsVersion{"1.5"});
    CHECK(model.getLanguage() == "de");
  }

  SUBCASE("Get non existing model non-strict mode with unknown language")
  {
    const auto& model = registry.getModel(rexsapi::TRexsVersion{"1.99"}, "es", false);
    CHECK(model.getVersion() == rexsapi::TRexsVersion{"1.5"});
    CHECK(model.getLanguage() == "en");
  }
}
