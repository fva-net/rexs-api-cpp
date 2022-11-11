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

#ifndef REXSAPI_MODEL_SAVER_HXX
#define REXSAPI_MODEL_SAVER_HXX

#include <rexsapi/JsonModelSerializer.hxx>
#include <rexsapi/JsonSerializer.hxx>
#include <rexsapi/Result.hxx>
#include <rexsapi/XMLModelSerializer.hxx>
#include <rexsapi/XMLSerializer.hxx>

namespace rexsapi
{
  /**
   * @brief The REXS model format to save a model in
   *
   */
  enum class TSaveType {
    JSON,  //!< Model shall be saved in JSON format
    XML    //!< Model shall be saved in XML format
  };


  /**
   * @brief Easy to use model saver convenience class abstracting REXS model store operations.
   *
   * Can store models in XML or JSON format to a file.
   *
   * Allows storing of multiple REXS model files with the same saver.
   */
  class TModelSaver
  {
  public:
    /**
     * @brief Stores a REXS TModel instance to the given filesystem path
     *
     * Models can be stored either in XML or JSON format. If the filesystem path does not contain an extension, the
     * store operation will add the correct extension depending on the desired store format. For XML format the
     * extension will be *.rexs* and for JSON *.rexsj*.
     *
     * @param result Describes the outcome of the store operation. Will contain messages upon issues encountered. If the
     * result yields false, the model was not stored.
     * @param model The REXS model to save
     * @param path The filesyatem path to store the model to
     * @param type The desired format to store the model in
     */
    void store(TResult& result, const TModel& model, const std::filesystem::path& path, TSaveType type) const noexcept
    {
      try {
        switch (type) {
          case TSaveType::JSON: {
            rexsapi::TJsonFileSerializer fileSerializer{addExtension(path, ".rexsj")};
            rexsapi::TJsonModelSerializer modelSerializer;
            modelSerializer.serialize(model, fileSerializer);
            break;
          }
          case TSaveType::XML: {
            rexsapi::TXMLFileSerializer xmlSerializer{addExtension(path, ".rexs")};
            rexsapi::XMLModelSerializer modelSerializer;
            modelSerializer.serialize(model, xmlSerializer);
            break;
          }
        }
      } catch (const std::exception& ex) {
        result.addError(TError{TErrorLevel::CRIT, fmt::format("cannot store model to {}: {}", ex.what())});
      }
    }

  private:
    static std::filesystem::path addExtension(std::filesystem::path path, std::string_view extension) noexcept
    {
      return path.has_extension() ? path : path.concat(extension);
    }
  };
}

#endif
