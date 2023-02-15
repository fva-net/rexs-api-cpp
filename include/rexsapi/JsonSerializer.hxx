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

#ifndef REXSAPI_JSON_SERIALIZER_HXX
#define REXSAPI_JSON_SERIALIZER_HXX

#include <rexsapi/Json.hxx>

#include <filesystem>
#include <fstream>

namespace rexsapi
{
  /**
   * @brief Outputs a json object into a string
   *
   */
  class TJsonStringSerializer
  {
  public:
    void serialize(const ordered_json& doc)
    {
      m_Model = doc.dump(2);
    }

    /**
     * @brief Returns the json string of the serialized model
     *
     * @return const std::string& to the json string
     */
    const std::string& getModel() const& noexcept
    {
      return m_Model;
    }

  private:
    std::string m_Model;
  };


  /**
   * @brief Outputs a json object into a file
   *
   */
  class TJsonFileSerializer
  {
  public:
    /**
     * @brief Constructs a new TJsonFileSerializer object
     *
     * @param file The filesystem path to save the json object to
     * @param indent The amount of indentation for nested structures. Set to -1 for the most compact format.
     * @throws TException if the directory of the given path does not exist
     */
    explicit TJsonFileSerializer(std::filesystem::path file, int indent = 2)
    : m_File{std::move(file)}
    , m_Indent{indent}
    {
      auto directory = m_File.parent_path();
      if (!std::filesystem::is_directory(directory)) {
        throw TException{fmt::format("{} is not a directory or does not exist", directory.string())};
      }
    }

    /**
     * @brief Outputs the json object into a file
     *
     * @param doc The json obejct containing the REXS json model
     * @throws TException if the file cannot be written
     */
    void serialize(const ordered_json& doc)
    {
      constexpr static uint8_t bom[] = {0xEF, 0xBB, 0xBF};
      std::ofstream stream{m_File};
      stream.write(reinterpret_cast<const char*>(bom), sizeof(bom));
      stream << doc.dump(m_Indent);
      stream.flush();
      if (!stream) {
        throw TException{fmt::format("Could not serialize model to {}", m_File.string())};
      }
    }

  private:
    std::filesystem::path m_File;
    int m_Indent;
  };
}

#endif
