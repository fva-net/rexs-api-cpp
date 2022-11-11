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

#include <rexsapi/JsonModelLoader.hxx>
#include <rexsapi/JsonModelSerializer.hxx>
#include <rexsapi/JsonSerializer.hxx>
#include <rexsapi/ModelLoader.hxx>
#include <rexsapi/ModelSaver.hxx>

#include <test/TemporaryDirectory.hxx>
#include <test/TestHelper.hxx>
#include <test/TestModel.hxx>
#include <test/TestModelLoader.hxx>

#include <doctest.h>

namespace
{
  class StringLoader
  {
  public:
    rexsapi::TModel load(rexsapi::TResult result, const std::string& buffer)
    {
      rexsapi::detail::TBufferModelLoader<rexsapi::TJsonSchemaValidator, rexsapi::TJsonModelLoader> loader{m_Validator,
                                                                                                           buffer};
      auto model = loader.load(rexsapi::TMode::STRICT_MODE, result, m_Registry);
      if (!model || !result) {
        throw rexsapi::TException{"cannot load model"};
      }
      return std::move(*model);
    }

  private:
    rexsapi::database::TModelRegistry m_Registry{createModelRegistry()};
    rexsapi::TFileJsonSchemaLoader m_SchemaLoader{projectDir() / "models" / "rexs-schema.json"};
    rexsapi::TJsonSchemaValidator m_Validator{m_SchemaLoader};
  };
}

TEST_CASE("Json serialize new model")
{
  const auto& dbModel = loadModel("1.4");
  rexsapi::TJsonModelSerializer modelSerializer;

  SUBCASE("Serialize model to memory")
  {
    rexsapi::TJsonStringSerializer stringSerializer;
    modelSerializer.serialize(createModel(dbModel), stringSerializer);
    REQUIRE_FALSE(stringSerializer.getModel().empty());
    StringLoader loader;
    rexsapi::TResult result;
    auto roundtripModel = loader.load(result, stringSerializer.getModel());
    CHECK(result);
    CHECK(roundtripModel.getInfo().getApplicationId() == "REXSApi Unit Test");
    CHECK(roundtripModel.getInfo().getApplicationVersion() == "1.0");
    CHECK(roundtripModel.getInfo().getDate() == "2022-05-20T08:59:10+01:00");
    CHECK(roundtripModel.getInfo().getVersion() == rexsapi::TRexsVersion{1, 4});
    REQUIRE(roundtripModel.getInfo().getApplicationLanguage().has_value());
    CHECK(*roundtripModel.getInfo().getApplicationLanguage() == "en");
    CHECK(roundtripModel.getComponents().size() == 7);
    CHECK(roundtripModel.getRelations().size() == 3);
    CHECK(roundtripModel.getLoadSpectrum().hasLoadCases());
    REQUIRE(roundtripModel.getLoadSpectrum().getLoadCases().size() == 1);
    CHECK(roundtripModel.getLoadSpectrum().getLoadCases()[0].getLoadComponents().size() == 2);
    REQUIRE(roundtripModel.getLoadSpectrum().hasAccumulation());
    REQUIRE(roundtripModel.getLoadSpectrum().getAccumulation().getLoadComponents().size() == 1);
    CHECK(roundtripModel.getLoadSpectrum().getAccumulation().getLoadComponents()[0].getLoadAttributes().size() == 2);
  }

  SUBCASE("Serialize model to file")
  {
    const auto registry = createModelRegistry();
    TemporaryDirectory guard;
    rexsapi::TJsonFileSerializer fileSerializer{guard.getTempDirectoryPath() / "test_model.rexsj"};
    modelSerializer.serialize(createModel(dbModel), fileSerializer);
    REQUIRE(std::filesystem::exists(guard.getTempDirectoryPath() / "test_model.rexsj"));

    rexsapi::TFileJsonSchemaLoader schemaLoader{projectDir() / "models" / "rexs-schema.json"};
    rexsapi::TJsonSchemaValidator validator{schemaLoader};

    rexsapi::detail::TFileModelLoader<rexsapi::TJsonSchemaValidator, rexsapi::TJsonModelLoader> loader{
      validator, guard.getTempDirectoryPath() / "test_model.rexsj"};
    rexsapi::TResult result;
    auto loadedModel = loader.load(rexsapi::TMode::STRICT_MODE, result, registry);
    CHECK(result);
    CHECK(loadedModel);
  }

  SUBCASE("Serialze model to file with model saver")
  {
    TemporaryDirectory guard;
    rexsapi::TResult result;
    rexsapi::TModelSaver{}.store(result, createModel(dbModel), guard.getTempDirectoryPath() / "test_model.rexsj",
                                 rexsapi::TSaveType::JSON);
    CHECK(result);
    REQUIRE(std::filesystem::exists(guard.getTempDirectoryPath() / "test_model.rexsj"));
    std::filesystem::remove(guard.getTempDirectoryPath() / "test_model.rexsj");
    rexsapi::TModelSaver{}.store(result, createModel(dbModel), guard.getTempDirectoryPath() / "test_model",
                                 rexsapi::TSaveType::JSON);
    CHECK(result);
    REQUIRE(std::filesystem::exists(guard.getTempDirectoryPath() / "test_model.rexsj"));
  }

  SUBCASE("Serialize to non existent directory")
  {
    CHECK_THROWS(rexsapi::TJsonFileSerializer{std::filesystem::path{"puschel"} / "test_model.rexsj"});
  }

  SUBCASE("Serialize stream error")
  {
    TemporaryDirectory guard;
    auto path = guard.getTempDirectoryPath() / "test";
    std::filesystem::create_directories(path);

    rexsapi::TJsonFileSerializer fileSerializer{path / "test_model.rexsj"};
    std::filesystem::remove(path);
    CHECK_THROWS(modelSerializer.serialize(createModel(dbModel), fileSerializer));
  }
}
