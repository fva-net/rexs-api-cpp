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

#ifndef REXSAPI_MODEL_LOADER_HXX
#define REXSAPI_MODEL_LOADER_HXX

#include <rexsapi/JsonModelLoader.hxx>
#include <rexsapi/XMLModelLoader.hxx>
#include <rexsapi/ZipArchive.hxx>
#include <rexsapi/database/FileResourceLoader.hxx>
#include <rexsapi/database/ModelRegistry.hxx>
#include <rexsapi/database/XMLModelLoader.hxx>

namespace rexsapi
{
  /**
   * @brief Creates a model registry containing all models found in the given filesystem path.
   *
   * The filesystem path should contain REXS database model files for different versions and languages. Additionally,
   * the XML database model schema file (*rexs-dbmodel.xsd*) has to be available in the path.
   *
   * @param path The filesystem path to load REXS database model files from.
   * @return database::TModelRegistry containing all available REXS database models
   * @throws TException if anything goes wrong with the creation of the registry, like missing permissions, no schema
   * found, parsing errors, etc.
   */
  static database::TModelRegistry createModelRegistry(const std::filesystem::path& path);


  /**
   * @brief Easy to use model loader convenience class abstracting REXS model load operations.
   *
   * All allowed kinds of REXS model files can be loaded trasparently with this loader. The loader will also create it's
   * own model registry. For the successful creation of the model registry, all schema files have to be available to
   * the loader. Additionally, all necessary REXS database model files for different versions and languages have to be
   * available. The REXS project contains the directory ```models``` with all relevant files that can be used with
   * the loader.
   *
   * Allows loading of multiple REXS model files with the same loader.
   */
  class TModelLoader
  {
  public:
    /**
     * @brief Constructs a new TModelLoader object.
     *
     * @param databasePath filesystem path containing REXS database model files for different versions and languages and
     * all relevant schema files. The necessary schema files are:
     * - rexs-dbmodel.xsd
     * - rexs-schema.xsd
     * - rexs-schema.json
     * @throws TException if the model registry or the schema validators cannot be created
     */
    explicit TModelLoader(const std::filesystem::path& databasePath)
    : m_Registry{createModelRegistry(databasePath)}
    , m_XMLSchemaValidator{createXMLSchemaValidator(databasePath)}
    , m_JsonValidator{createJsonSchemaValidator(databasePath)}
    {
    }

    /**
     * @brief Constructs a new TModelLoader object.
     *
     * @param databasePath filesystem path containing REXS database model files for different versions and languages and
     * all relevant schema files. The necessary schema files are:
     * - rexs-dbmodel.xsd
     * - rexs-schema.xsd
     * - rexs-schema.json
     * @param customExtensionMappings Additional custom extension mappings to respect for REXS model file type detection
     * @throws TException if the model registry or the schema validators cannot be created
     */
    TModelLoader(const std::filesystem::path& databasePath, TCustomExtensionMappings customExtensionMappings)
    : m_Registry{createModelRegistry(databasePath)}
    , m_XMLSchemaValidator{createXMLSchemaValidator(databasePath)}
    , m_JsonValidator{createJsonSchemaValidator(databasePath)}
    , m_ExtensionChecker{std::move(customExtensionMappings)}
    {
    }

    /**
     * @brief Loads a RESX model file and creates a TModel instance.
     *
     * The given file can contain a REXS model in XML or JSON format. The files extension has to be one of the allowed
     * extensions:
     * - .rexs
     * - .rexs.zip
     * - .rexsj
     * - .rexs.json
     * - .rexsz
     * - .rexs.zip
     *
     * In case of a zip file, the archive is searched for a REXS model file with one of the above mentioned extensions.
     *
     * The result will describe the outcome of the load operation. In case of a critical error, the result will yield
     * false and there will be no TModel. It is perfectly ok that the result yields false, but a TModel instance was
     * created. That outcome is the case when loading non-compliant models. Using the TMode::RELEAXED mode,
     * non-compliant REXS model can be loaded without the result yielding false even in case of issues.
     *
     * @param path The filesystem path to the REXS model file to load
     * @param result Describes the outcome of the load operation. Will contain messages upon issues encountered.
     * @param mode Defines how to handle encountered issues while processing a REXS model file
     * @return std::optional<TModel> containing a TModel instance if the model could be loaded susscessful. May contain
     * a TModel instance even in case of issues encountered while loading.
     */
    std::optional<TModel> load(const std::filesystem::path& path, TResult& result,
                               TMode mode = TMode::STRICT_MODE) const noexcept;

  private:
    static TXSDSchemaValidator createXMLSchemaValidator(const std::filesystem::path& path);

    static TJsonSchemaValidator createJsonSchemaValidator(const std::filesystem::path& path);

    database::TModelRegistry m_Registry;
    const TXSDSchemaValidator m_XMLSchemaValidator;
    const TJsonSchemaValidator m_JsonValidator;
    const TExtensionChecker m_ExtensionChecker;
  };


  namespace detail
  {
    template<typename TSchemaValidator, typename TLoader>
    class TFileModelLoader
    {
    public:
      explicit TFileModelLoader(const TSchemaValidator& validator, std::filesystem::path path)
      : m_Validator{validator}
      , m_Path{std::move(path)}
      {
      }

      [[nodiscard]] std::optional<TModel> load(TMode mode, TResult& result,
                                               const rexsapi::database::TModelRegistry& registry);

    private:
      const TSchemaValidator& m_Validator;
      std::filesystem::path m_Path;
    };


    template<typename TSchemaValidator, typename TLoader>
    class TBufferModelLoader
    {
    public:
      explicit TBufferModelLoader(const TSchemaValidator& validator, const std::string& buffer)
      : m_Validator{validator}
      , m_Buffer{buffer.begin(), buffer.end()}
      {
      }

      explicit TBufferModelLoader(const TSchemaValidator& validator, std::vector<uint8_t> buffer)
      : m_Validator{validator}
      , m_Buffer{std::move(buffer)}
      {
      }

      [[nodiscard]] std::optional<TModel> load(TMode mode, TResult& result,
                                               const rexsapi::database::TModelRegistry& registry);

    private:
      const TSchemaValidator& m_Validator;
      std::vector<uint8_t> m_Buffer;
    };
  }

  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  static inline database::TModelRegistry createModelRegistry(const std::filesystem::path& path)
  {
    TFileXsdSchemaLoader schemaLoader{path / "rexs-dbmodel.xsd"};
    database::TFileResourceLoader resourceLoader{path};
    database::TXmlModelLoader modelLoader{resourceLoader, schemaLoader};
    return database::TModelRegistry::createModelRegistry(modelLoader).first;
  }

  inline std::optional<TModel> TModelLoader::load(const std::filesystem::path& path, TResult& result,
                                                  TMode mode) const noexcept
  {
    std::optional<TModel> model;
    result.reset();

    try {
      switch (m_ExtensionChecker.getFileType(path)) {
        case TFileType::XML: {
          detail::TFileModelLoader<TXSDSchemaValidator, TXMLModelLoader> loader{m_XMLSchemaValidator, path};
          model = loader.load(mode, result, m_Registry);
          break;
        }
        case TFileType::JSON: {
          detail::TFileModelLoader<TJsonSchemaValidator, TJsonModelLoader> loader{m_JsonValidator, path};
          model = loader.load(mode, result, m_Registry);
          break;
        }
        case TFileType::COMPRESSED: {
          try {
            detail::ZipArchive archive{path, m_ExtensionChecker};
            auto [buffer, type] = archive.load();
            if (type == TFileType::XML) {
              detail::TBufferModelLoader<TXSDSchemaValidator, TXMLModelLoader> loader{m_XMLSchemaValidator,
                                                                                      std::move(buffer)};
              model = loader.load(mode, result, m_Registry);
            } else if (type == TFileType::JSON) {
              detail::TBufferModelLoader<TJsonSchemaValidator, TJsonModelLoader> loader{m_JsonValidator,
                                                                                        std::move(buffer)};
              model = loader.load(mode, result, m_Registry);
            }
          } catch (const std::exception& ex) {
            result.addError(TError{TErrorLevel::CRIT,
                                   fmt::format("compressed file {} cannot be loaded: {}", path.string(), ex.what())});
          }
          break;
        }
        default:
          result.addError(
            TError{TErrorLevel::CRIT, fmt::format("extension {} currently not supported", path.extension().string())});
      }
    } catch (const std::exception& ex) {
      result.addError(TError{TErrorLevel::CRIT, fmt::format("cannot load model: {}", ex.what())});
    }

    return model;
  }

  inline TXSDSchemaValidator TModelLoader::createXMLSchemaValidator(const std::filesystem::path& path)
  {
    TFileXsdSchemaLoader schemaLoader{path / "rexs-schema.xsd"};
    return TXSDSchemaValidator{schemaLoader};
  }

  inline TJsonSchemaValidator TModelLoader::createJsonSchemaValidator(const std::filesystem::path& path)
  {
    TFileJsonSchemaLoader schemaLoader{path / "rexs-schema.json"};
    return TJsonSchemaValidator{schemaLoader};
  }

  template<typename TSchemaValidator, typename TLoader>
  inline std::optional<TModel>
  detail::TBufferModelLoader<TSchemaValidator, TLoader>::load(TMode mode, TResult& result,
                                                              const rexsapi::database::TModelRegistry& registry)
  {
    TLoader loader{mode, m_Validator};
    return loader.load(result, registry, m_Buffer);
  }

  template<typename TSchemaValidator, typename TLoader>
  inline std::optional<TModel>
  detail::TFileModelLoader<TSchemaValidator, TLoader>::load(TMode mode, TResult& result,
                                                            const rexsapi::database::TModelRegistry& registry)
  {
    auto buffer = detail::loadFile(result, m_Path);
    if (!result) {
      return {};
    }
    return TLoader{mode, m_Validator}.load(result, registry, buffer);
  }
}

#endif
