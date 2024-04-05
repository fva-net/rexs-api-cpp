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

#ifndef REXSAPI_DATABASE_MODEL_REGISTRY_HXX
#define REXSAPI_DATABASE_MODEL_REGISTRY_HXX

#include <rexsapi/database/Model.hxx>


/** @file */

namespace rexsapi::database
{
  /**
   * @brief The database model registry contains all REXS database model versions.
   *
   * It contains one REXS database model for each version and language combination loaded into it. The registry has to
   * be used to retrieve REXS database models for a specfic version. The database model can then be used to create or
   * import models, or to just query it for information.
   */
  class TModelRegistry
  {
  public:
    ~TModelRegistry() = default;

    TModelRegistry(const TModelRegistry&) = delete;
    TModelRegistry& operator=(const TModelRegistry&) = delete;
    TModelRegistry(TModelRegistry&&) noexcept = default;
    TModelRegistry& operator=(TModelRegistry&&) = delete;

    /**
     * @brief Retrieves a database model for a specific version and language.
     *
     * @param version The database model version to retrieve
     * @param language The language of the database model to retrieve
     * @param strict Determines if the latest available model version shall be returned if an exact one was not found.
     * If set to true, will throw an exception if the exact version could not be found.
     * @return const TModel& to the found database model
     * @throws TException if the specific version or language is not available and strict mode was set to true
     */
    [[nodiscard]] const TModel& getModel(const TRexsVersion& version, const std::string& language,
                                         bool strict = true) const;

    /**
     * @brief Creates a model registry.
     *
     * @tparam TModelLoader Loader class for loading the schema and database model files. The TXmlModelLoader class is
     * provided as default implementation. The loader has to define the following method <br> ```TResult load(const
     * std::function<void(TModel)>& callback) const```
     * @param loader The loader instance to load the schema and database model files with
     * @return std::pair<TModelRegistry, TResult> containing the model registry filled with the loaded models and a
     * result describing the outcome of the loading. In case of a failure, the registry will contain no or only a subset
     * of the database models.
     */
    template<typename TModelLoader>
    static std::pair<TModelRegistry, TResult> createModelRegistry(const TModelLoader& loader);

  private:
    explicit TModelRegistry(std::vector<TModel>&& models)
    : m_Models{std::move(models)}
    {
    }

    std::vector<TModel> m_Models;
  };


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  inline const TModel& TModelRegistry::getModel(const TRexsVersion& version, const std::string& language,
                                                bool strict) const
  {
    const auto it = std::find_if(m_Models.begin(), m_Models.end(), [&version, &language](const auto& model) {
      return model.getVersion() == version && model.getLanguage() == language;
    });

    if (it != m_Models.end()) {
      return *it;
    }

    if (!strict) {
      // no exact model was found, find the latest model for the given language
      // otherwise choose english
      const TRexsVersion baseVersion{1, 0};
      const TModel* baseModel{nullptr};
      std::for_each(m_Models.begin(), m_Models.end(), [&baseModel, &baseVersion, &language](const auto& model) mutable {
        if (model.getLanguage() == language && model.getVersion() >= (baseModel ? baseModel->getVersion() : baseVersion)) {
          baseModel = &model;
        }
        if (model.getLanguage() == "en" && model.getVersion() > (baseModel ? baseModel->getVersion() : baseVersion)) {
          baseModel = &model;
        }
      });

      if (baseModel) {
        return *baseModel;
      }
    }

    throw TException{
      fmt::format("cannot find a database model for version '{}' and locale '{}'", version.asString(), language)};
  }

  template<typename TModelLoader>
  std::pair<TModelRegistry, TResult> TModelRegistry::createModelRegistry(const TModelLoader& loader)
  {
    std::vector<TModel> models;
    auto result = loader.load([&models](TModel&& model) {
      models.emplace_back(std::move(model));
    });

    return std::make_pair(TModelRegistry{std::move(models)}, result);
  }
}

#endif
