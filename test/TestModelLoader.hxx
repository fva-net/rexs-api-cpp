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
#include <rexsapi/database/ModelRegistry.hxx>
#include <rexsapi/database/XMLModelLoader.hxx>

#include <test/TestHelper.hxx>


static inline std::vector<rexsapi::database::TModel> loadModels()
{
  std::vector<rexsapi::database::TModel> models;
  rexsapi::TFileXsdSchemaLoader schemaLoader{projectDir() / "models" / "rexs-dbmodel.xsd"};
  rexsapi::database::TFileResourceLoader loader{projectDir() / "models"};
  rexsapi::database::TXmlModelLoader modelLoader{loader, schemaLoader};
  auto result = modelLoader.load([&models](rexsapi::database::TModel&& model) {
    models.emplace_back(std::move(model));
  });

  return models;
}

static inline const rexsapi::database::TModel& loadModel(const std::string& version)
{
  static auto models = loadModels();
  auto it = std::find_if(models.begin(), models.end(), [&version](const auto& model) {
    return rexsapi::TRexsVersion{version} == model.getVersion() && model.getLanguage() == "en";
  });

  if (it == models.end()) {
    throw rexsapi::TException{"no model with version '" + version + "' found"};
  }
  return *it;
}

static inline rexsapi::database::TModelRegistry createModelRegistry()
{
  rexsapi::TFileXsdSchemaLoader schemaLoader{projectDir() / "models" / "rexs-dbmodel.xsd"};
  rexsapi::database::TFileResourceLoader resourceLoader{projectDir() / "models"};
  rexsapi::database::TXmlModelLoader modelLoader{resourceLoader, schemaLoader};
  return rexsapi::database::TModelRegistry::createModelRegistry(modelLoader).first;
}
