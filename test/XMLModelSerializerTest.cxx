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
#include <rexsapi/ModelSaver.hxx>
#include <rexsapi/XMLModelLoader.hxx>
#include <rexsapi/XMLModelSerializer.hxx>
#include <rexsapi/XMLSerializer.hxx>

#include <test/TemporaryDirectory.hxx>
#include <test/TestHelper.hxx>
#include <test/TestModel.hxx>
#include <test/TestModelLoader.hxx>

#include <doctest.h>

namespace
{
  class FileLoader
  {
  public:
    rexsapi::TModel load(const std::filesystem::path& modelFile, rexsapi::TMode mode = rexsapi::TMode::STRICT_MODE)
    {
      rexsapi::detail::TFileModelLoader<rexsapi::TXSDSchemaValidator, rexsapi::TXMLModelLoader> loader{m_Validator,
                                                                                                       modelFile};
      rexsapi::TResult result;
      auto model = loader.load(mode, result, m_Registry);
      if (!model || !result) {
        throw rexsapi::TException{"cannot load model"};
      }
      return std::move(*model);
    }

  private:
    rexsapi::database::TModelRegistry m_Registry{createModelRegistry()};
    rexsapi::TFileXsdSchemaLoader m_SchemaLoader{projectDir() / "models" / "rexs-file.xsd"};
    rexsapi::TXSDSchemaValidator m_Validator{m_SchemaLoader};
  };

  class StringLoader
  {
  public:
    rexsapi::TModel load(const std::string& buffer)
    {
      rexsapi::detail::TBufferModelLoader<rexsapi::TXSDSchemaValidator, rexsapi::TXMLModelLoader> loader{m_Validator,
                                                                                                         buffer};
      rexsapi::TResult result;
      auto model = loader.load(rexsapi::TMode::STRICT_MODE, result, m_Registry);
      if (!model || !result) {
        throw rexsapi::TException{"cannot load model"};
      }
      return std::move(*model);
    }

  private:
    rexsapi::database::TModelRegistry m_Registry{createModelRegistry()};
    rexsapi::TFileXsdSchemaLoader m_SchemaLoader{projectDir() / "models" / "rexs-file.xsd"};
    rexsapi::TXSDSchemaValidator m_Validator{m_SchemaLoader};
  };
}

TEST_CASE("XML model file serializer test")
{
  FileLoader loader;
  auto model =
    loader.load(projectDir() / "test" / "example_models" / "FVA_worm_stage_1-4.rexs", rexsapi::TMode::RELAXED_MODE);

  SUBCASE("Serialize loaded model")
  {
    TemporaryDirectory tmpDir;
    rexsapi::TXMLFileSerializer xmlSerializer{tmpDir.getTempDirectoryPath() / "FVA_worm_stage_1-4.rexs"};
    rexsapi::XMLModelSerializer modelSerializer;
    modelSerializer.serialize(model, xmlSerializer);
    CHECK(std::filesystem::exists(tmpDir.getTempDirectoryPath() / "FVA_worm_stage_1-4.rexs"));
    auto roundtripModel =
      loader.load(tmpDir.getTempDirectoryPath() / "FVA_worm_stage_1-4.rexs", rexsapi::TMode::RELAXED_MODE);
    CHECK(roundtripModel.getComponents().size() == model.getComponents().size());
    CHECK(roundtripModel.getRelations().size() == model.getRelations().size());
  }
}

TEST_CASE("XML serialize new model")
{
  const auto& dbModel = loadModel("1.5");
  rexsapi::TXMLStringSerializer stringSerializer;
  rexsapi::XMLModelSerializer modelSerializer;

  SUBCASE("Serialize model")
  {
    modelSerializer.serialize(createModel(dbModel), stringSerializer);
    CHECK_FALSE(stringSerializer.getModel().empty());
    StringLoader loader;
    auto roundtripModel = loader.load(stringSerializer.getModel());
    CHECK(roundtripModel.getInfo().getApplicationId() == "REXSApi Unit Test");
    CHECK(roundtripModel.getInfo().getApplicationVersion() == "1.0");
    CHECK(roundtripModel.getInfo().getDate() == "2022-05-20T08:59:10+01:00");
    CHECK(roundtripModel.getInfo().getVersion() == rexsapi::TRexsVersion{1, 5});
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

  SUBCASE("Serialze model to file with model saver")
  {
    TemporaryDirectory guard;
    rexsapi::TResult result;
    rexsapi::TModelSaver{}.store(result, createModel(dbModel), guard.getTempDirectoryPath() / "test_model.rexs",
                                 rexsapi::TSaveType::XML);
    CHECK(result);
    REQUIRE(std::filesystem::exists(guard.getTempDirectoryPath() / "test_model.rexs"));
  }
}
