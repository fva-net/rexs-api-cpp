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

#ifndef REXSAPI_FILE_TYPES_HXX
#define REXSAPI_FILE_TYPES_HXX

#include <rexsapi/ConversionHelper.hxx>
#include <rexsapi/Format.hxx>

#include <filesystem>
#include <vector>

namespace rexsapi
{
  /**
   * @brief Represents a specific REXS file type.
   *
   */
  enum class TFileType {
    UNKNOWN,    //!< The file type is unkown and cannot be processed
    XML,        //!< The file type is REXS XML
    JSON,       //!< The file type is REXS JSON
    COMPRESSED  //!< The file type is compressed and has to be unzipped before real type can be extracted
  };

  /**
   * @brief Creates a TFileType from a string.
   *
   * @param type The string to convert
   * @return TFileType corresponding to the given string
   * @throws TException if the type is not a known one
   */
  TFileType fileTypeFromString(const std::string& type);


  /**
   * @brief Defines mappings for custom file extensions to REXS file types.
   *
   * Currently, only REXS XML and JSON are allowed formats.
   *
   */
  struct TCustomExtensionMapping {
    std::string m_Extension;  //!< The custom extension to use for detecting the REXS file type. Must contain the
                              //!< starting dot of the extension, e.g. ".rexs.in".
    TFileType m_Type;         //!< The REXS file type the specified extension shall be loaded with.
  };

  using TCustomExtensionMappings = std::vector<TCustomExtensionMapping>;


  /**
   * @brief Determines the REXS file type from a given REXS model filesystem path.
   *
   */
  class TExtensionChecker
  {
  public:
    TExtensionChecker() = default;

    /**
     * @brief Constructs a new TExtensionChecker object.
     *
     * @param customMappings Defines additional custom extensions to use for REXS model file type detection
     */
    explicit TExtensionChecker(TCustomExtensionMappings customMappings)
    : m_CustomMappings{std::move(customMappings)}
    {
    }

    /**
     * @brief Returns the file type from a REXS model filesystem path.
     *
     * Will look for all allowed standard REXS extensions: .rexs, .rexsj, .rexsz, .rexs.xml, .rexs.json, .rexs.zip.
     * If no standard extension qualifies, will try custom extensions if specified.
     * The path is allowed to contain additional '.' characters before the extension begins. E.g.
     * "model_file.some_other_text.rexs.json"
     *
     * @param path The filesystem path to determine the file type for
     * @return TFileType determined. Can be TFileType::UKNOWN.
     */
    TFileType getFileType(const std::filesystem::path& path) const noexcept;

  private:
    TCustomExtensionMappings m_CustomMappings{};
  };


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  inline TFileType fileTypeFromString(const std::string& type)
  {
    if (rexsapi::toupper(type) == "XML") {
      return TFileType::XML;
    }
    if (rexsapi::toupper(type) == "JSON") {
      return TFileType::JSON;
    }

    throw TException{fmt::format("unknown file type {}", type)};
  }


  inline TFileType TExtensionChecker::getFileType(const std::filesystem::path& path) const noexcept
  {
    auto extension = path.extension();
    if (path.stem().has_extension()) {
      auto stem_extention = path.stem().extension();
      extension = stem_extention.replace_extension(extension);
    }

    /// Attention: deliberately using path.extension() here and *not* extension
    if (path.extension() == ".rexs" || extension == ".rexs.xml") {
      return TFileType::XML;
    }
    if (path.extension() == ".rexsz" || extension == ".rexs.zip") {
      return TFileType::COMPRESSED;
    }
    if (path.extension() == ".rexsj" || extension == ".rexs.json") {
      return TFileType::JSON;
    }
    for (const auto& mapping : m_CustomMappings) {
      if (path.extension() == mapping.m_Extension || extension == mapping.m_Extension) {
        return mapping.m_Type;
      }
    }

    return TFileType::UNKNOWN;
  }
}

#endif
