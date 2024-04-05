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

#ifndef REXSAPI_JSON_SCHEMA_VALIDATOR_HXX
#define REXSAPI_JSON_SCHEMA_VALIDATOR_HXX

#include <rexsapi/Exception.hxx>
#include <rexsapi/FileUtils.hxx>
#include <rexsapi/Format.hxx>
#include <rexsapi/Json.hxx>

#include <filesystem>
#define VALIJSON_USE_EXCEPTIONS 1
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/utils/nlohmann_json_utils.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>
#undef VALIJSON_USE_EXCEPTIONS

namespace rexsapi
{
  /**
   * @brief Load a json schema from a filesystem path.
   *
   */
  class TFileJsonSchemaLoader
  {
  public:
    /**
     * @brief Constructs a new TFileJsonSchemaLoader object.
     *
     * @param jsonFile The filesystem path to the json schema
     */
    explicit TFileJsonSchemaLoader(std::filesystem::path jsonFile)
    : m_JsonFile{std::move(jsonFile)}
    {
    }

    /**
     * @brief Loads the json schema from the specfified path.
     *
     * @return json object of the json schema
     * @throws TException if the schema cannot be loaded or parsed
     */
    [[nodiscard]] json load() const;

  private:
    std::filesystem::path m_JsonFile;
  };


  /**
   * @brief Load a json schema from a string.
   *
   */
  class TBufferJsonSchemaLoader
  {
  public:
    /**
     * @brief Constructs a new TBufferJsonSchemaLoader object
     *
     * @param jsonSchema The string containing the json schema
     */
    explicit TBufferJsonSchemaLoader(std::string_view jsonSchema)
    : m_JsonSchema{jsonSchema}
    {
    }

    /**
     * @brief Loads the json schema from the specfified string.
     *
     * @return json object of the json schema
     * @throws TException if the schema cannot be parsed
     */
    [[nodiscard]] json load() const;

  private:
    std::string m_JsonSchema;
  };


  /**
   * @brief Validates a json document with a json schema.
   *
   */
  class TJsonSchemaValidator
  {
  public:
    /**
     * @brief Constructs a new TJsonSchemaValidator object.
     *
     * @tparam TJsonSchemaLoader Loader class for loading the json schema. The TFileJsonSchemaLoader and
     * TBufferJsonSchemaLoader classes are provided as default implementations. The loader class has to define the
     * following method <br> ```json load() const```
     * @param loader The loader instance to load the json schema with
     * @throws TException if the schema could not be loaded or processed
     */
    template<typename TJsonSchemaLoader>
    explicit TJsonSchemaValidator(const TJsonSchemaLoader& loader)
    {
      const auto doc = loader.load();
      valijson::SchemaParser parser;
      valijson::adapters::NlohmannJsonAdapter schemaDocumentAdapter(doc);

      try {
        parser.populateSchema(schemaDocumentAdapter, m_Schema);
      } catch (const std::exception& ex) {
        throw TException{fmt::format("Cannot populate schema: {}", ex.what())};
      }
    }

    /**
     * @brief Validates the given json document against the configured json schema.
     *
     * @param doc The json document to validate
     * @param errors Will be filled with issues encountered while validating the document
     * @return true if the validation was successful
     * @return false if the validation failed
     */
    [[nodiscard]] bool validate(const json& doc, std::vector<std::string>& errors) const;

  private:
    valijson::Schema m_Schema;
  };

  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  inline json TFileJsonSchemaLoader::load() const
  {
    TResult result;
    const auto buffer = detail::loadFile(result, m_JsonFile);
    if (!result) {
      throw TException{
        fmt::format("Cannot load json schema '{}': {}", m_JsonFile.string(), result.getErrors()[0].getMessage())};
    }

    json doc;
    try {
      doc = json::parse(buffer);
    } catch (const std::exception& ex) {
      throw TException{fmt::format("Cannot parse json schema '{}': {}", m_JsonFile.string(), ex.what())};
    }

    return doc;
  }

  inline json TBufferJsonSchemaLoader::load() const
  {
    json doc;

    try {
      doc = json::parse(m_JsonSchema);
    } catch (const std::exception& ex) {
      throw TException{fmt::format("Cannot parse json schema: {}", ex.what())};
    }

    return doc;
  }

  inline bool TJsonSchemaValidator::validate(const json& doc, std::vector<std::string>& errors) const
  {
    valijson::Validator validator(valijson::Validator::kStrongTypes);
    valijson::ValidationResults results;
    valijson::adapters::NlohmannJsonAdapter targetDocumentAdapter(doc);
    if (!validator.validate(m_Schema, targetDocumentAdapter, &results)) {
      valijson::ValidationResults::Error error;
      unsigned int errorNum = 0;
      while (results.popError(error)) {
        std::string context;
        auto it = error.context.begin();
        for (; it != error.context.end(); it++) {
          context += *it;
        }
        errors.emplace_back(fmt::format("Error #{} context: {} desc: {}", ++errorNum, context, error.description));
        ++errorNum;
      }
    }

    return errors.empty();
  }
}

#endif
