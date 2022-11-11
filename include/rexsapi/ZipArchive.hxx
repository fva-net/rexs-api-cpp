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

#ifndef REXSAPI_ZIP_ARCHIVE_HXX
#define REXSAPI_ZIP_ARCHIVE_HXX

#include <rexsapi/FileTypes.hxx>

#define MINIZ_NO_ZLIB_APIS
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#ifndef REXSAPI_MINIZ_IMPL
  #define MINIZ_HEADER_FILE_ONLY
#endif
#include <filesystem>
#if defined(_MSC_VER)
  #pragma warning(push)
  #pragma warning(disable : 4334 4127)
#endif
#include <miniz/miniz.h>
#if defined(_MSC_VER)
  #pragma warning(pop)
#endif
#include <vector>

namespace rexsapi::detail
{
  class ZipArchive
  {
  public:
    explicit ZipArchive(std::filesystem::path archive, const TExtensionChecker& extensionChecker);

    ZipArchive(const ZipArchive&) = delete;
    ZipArchive(ZipArchive&&) = delete;
    ZipArchive& operator=(const ZipArchive&) = delete;
    ZipArchive& operator=(ZipArchive&&) = delete;

    ~ZipArchive()
    {
      mz_zip_reader_end(&m_ZipArchive);
    }

    std::pair<std::vector<uint8_t>, TFileType> load();

  private:
    std::filesystem::path m_Archive;
    const TExtensionChecker& m_ExtensionChecker;
    mz_zip_archive m_ZipArchive;
    mz_uint m_FileIndex{0};
    TFileType m_Type{TFileType::UNKNOWN};
  };


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  inline ZipArchive::ZipArchive(std::filesystem::path archive, const TExtensionChecker& extensionChecker)
  : m_Archive{std::move(archive)}
  , m_ExtensionChecker{extensionChecker}
  {
    ::memset(&m_ZipArchive, 0, sizeof(m_ZipArchive));
    if (mz_zip_reader_init_file(&m_ZipArchive, m_Archive.string().c_str(), 0) == MZ_FALSE) {
      throw TException{fmt::format("Cannot open zip archive '{}'", m_Archive.string())};
    }
    for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&m_ZipArchive); ++i) {
      mz_zip_archive_file_stat file_stat;
      if (mz_zip_reader_file_stat(&m_ZipArchive, i, &file_stat) == MZ_FALSE) {
        throw TException{fmt::format("Cannot open zip archive '{}'", m_Archive.string())};
      }
      m_Type = m_ExtensionChecker.getFileType(file_stat.m_filename);
      if (m_Type == TFileType::UNKNOWN) {
        continue;
      }
      m_FileIndex = i;
      break;
    }
    if (m_Type == TFileType::UNKNOWN) {
      throw TException{fmt::format("No rexs file in zip archive '{}'", m_Archive.string())};
    }
  }

  inline std::pair<std::vector<uint8_t>, TFileType> ZipArchive::load()
  {
    std::vector<uint8_t> buffer;
    size_t uncompressedSize = 0;
    auto* p = mz_zip_reader_extract_to_heap(&m_ZipArchive, m_FileIndex, &uncompressedSize, 0);
    if (p == nullptr) {
      throw TException{fmt::format("Cannot extract rexs file from zip archive '{}': {}", m_Archive.string(),
                                   mz_zip_get_error_string(m_ZipArchive.m_last_error))};
    }
    buffer.resize(uncompressedSize);
    ::memcpy(buffer.data(), p, uncompressedSize);
    mz_free(p);
    return std::make_pair(std::move(buffer), m_Type);
  }
}

#endif
