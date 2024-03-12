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

#include <rexsapi/database/FileResourceLoader.hxx>
#include <rexsapi/database/XMLModelLoader.hxx>

#include <test/TestHelper.hxx>

#include <set>

#include <doctest.h>


namespace
{
  class StringResourceLoader
  {
  public:
    explicit StringResourceLoader(std::string buffer)
    : m_Buffer{std::move(buffer)}
    {
    }

    rexsapi::TResult load(const std::function<void(rexsapi::TResult&, std::vector<uint8_t>&)>& callback) const
    {
      if (!callback) {
        throw rexsapi::TException{"callback not set for resource loader"};
      }

      rexsapi::TResult result;
      std::vector<uint8_t> buf{m_Buffer.begin(), m_Buffer.end()};
      callback(result, buf);

      return result;
    }

  private:
    std::string m_Buffer;
  };
}

TEST_CASE("XML database model loader test")
{
  rexsapi::TFileXsdSchemaLoader schemaLoader{projectDir() / "models" / "rexs-dbmodel.xsd"};
  std::vector<rexsapi::database::TModel> models;

  SUBCASE("Load existing models")
  {
    rexsapi::database::TFileResourceLoader loader{projectDir() / "models"};

    rexsapi::database::TXmlModelLoader modelLoader{loader, schemaLoader};
    auto result = modelLoader.load([&models](rexsapi::database::TModel&& model) {
      models.emplace_back(std::move(model));
    });

    CHECK(result);
    REQUIRE(models.size() == 14);
    std::set<std::string, std::less<>> languages;
    for (const auto& model : models) {
      languages.insert(model.getLanguage());
    }
    CHECK(languages.size() == 2);
    CHECK(languages.find("de") != languages.end());
    CHECK(languages.find("en") != languages.end());

    auto it = std::find_if(models.begin(), models.end(), [](const auto& model) {
      return model.getVersion() == rexsapi::TRexsVersion{1, 4};
    });
    REQUIRE(it != models.end());

    const auto& attribute = it->findAttributeById("bulk_temperature");
    REQUIRE(attribute.getInterval().has_value());
    CHECK_FALSE(attribute.getInterval()->check(-273.16));
    CHECK(attribute.getInterval()->check(0));
  }

  SUBCASE("Load broken XML")
  {
    const auto* s =
      R"(<rexsModel version=" 1.4 " status=" RELEASED " language=" de " createdDate="2022-04-19T13:03:21.881+02:00"><units></rexsModel>)";
    StringResourceLoader loader{s};
    rexsapi::database::TXmlModelLoader modelLoader{loader, schemaLoader};
    auto result = modelLoader.load([&models](rexsapi::database::TModel&& model) {
      models.emplace_back(std::move(model));
    });
    CHECK_FALSE(result);
    CHECK(models.empty());
  }
}
