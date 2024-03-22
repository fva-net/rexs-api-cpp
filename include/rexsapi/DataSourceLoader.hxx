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

#ifndef REXSAPI_DATA_SOURCE_LOADER_HXX
#define REXSAPI_DATA_SOURCE_LOADER_HXX

#include <rexsapi/DataSourceResolver.hxx>
#include <rexsapi/ModelLoader.hxx>

#include <filesystem>


namespace rexsapi
{
  /**
   * @brief File based loader for resolving data sources to models.
   *
   * Works exactly like a TModelLoader and has the same restrictions.
   */
  class TDataSourceLoader : public TDataSourceResolver
  {
  public:
    /**
     * @brief Constructs a new TDataSourceLoader object.
     *
     * @param databasePath filesystem path containing REXS database model files for different versions and languages and
     * all relevant schema files. Needs exactly the same files as the TModelLoader.
     * @param path The filesystem path used as base directory for the REXS model files to load
     */
    explicit TDataSourceLoader(const std::filesystem::path& databasePath, const std::filesystem::path& path)
    : m_Loader{databasePath, this}
    , m_Path{path}
    {
    }

    /**
     * @brief Loads a model from a data source.
     *
     * Internally, a TModelLoader is used to load the model.
     *
     * @param data_source The REXS model file to load. Will be prefixed by the databasePath given in the constructor.
     * @param result Describes the outcome of the operation. Will contain messages upon issues encountered.
     * @param mode Defines how to handle encountered issues while loading the model
     * @return The loaded model if successful, otherwise an empty optional. The result will reflect all issues
     *         encountered during the load operation.
     */
    std::optional<TModel> load(const std::string& data_source, TResult& result,
                               TMode mode = TMode::STRICT_MODE) const override;

  private:
    TModelLoader m_Loader;
    const std::filesystem::path m_Path;
  };

  std::optional<TModel> TDataSourceLoader::load(const std::string& data_source, TResult& result, TMode mode) const
  {
    return m_Loader.load(m_Path / data_source, result, mode);
  }
}

#endif
