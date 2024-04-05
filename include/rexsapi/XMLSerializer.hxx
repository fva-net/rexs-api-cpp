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

#ifndef REXSAPI_XML_SERIALIZER_HXX
#define REXSAPI_XML_SERIALIZER_HXX

#include <rexsapi/Xml.hxx>

namespace rexsapi
{
  /**
   * @brief Outputs an xml object into a file.
   *
   */
  class TXMLFileSerializer
  {
  public:
    /**
     * @brief Constructs a new TXMLFileSerializer object
     *
     * @param file The filesystem path to save the xml object to
     * @throws TException if the directory of the given path does not exist
     */
    explicit TXMLFileSerializer(std::filesystem::path file)
    : m_File{std::move(file)}
    {
      auto directory = m_File.parent_path();
      if (!std::filesystem::is_directory(directory)) {
        throw TException{fmt::format("'{}' is not a directory or does not exist", directory.string())};
      }
    }

    /**
     * @brief Outputs the xml object into a file.
     *
     * Will add a UTF-8 BOM to the beginning of the file.
     *
     * @param doc The xml object containing the REXS xml model
     */
    void serialize(const pugi::xml_document& doc) const
    {
      if (!doc.save_file(m_File.c_str(), "  ", pugi::format_indent | pugi::format_write_bom, pugi::encoding_utf8)) {
        throw TException{fmt::format("could not serialize '{}'", m_File.string())};
      }
    }

  private:
    std::filesystem::path m_File;
  };


  /**
   * @brief Outputs an xml object into a string.
   *
   * Will add a UTF-8 BOM to the beginning of the buffer.
   *
   */
  class TXMLStringSerializer
  {
  public:
    void serialize(const pugi::xml_document& doc)
    {
      std::stringstream stream;
      doc.save(stream, "  ", pugi::format_indent | pugi::format_write_bom, pugi::encoding_utf8);
      m_Model = stream.str();
    }

    /**
     * @brief Returns the xml string of the serialized model.
     *
     * @return const std::string& to the xml string
     */
    const std::string& getModel() const& noexcept
    {
      return m_Model;
    }

  private:
    std::string m_Model;
  };
}

#endif
