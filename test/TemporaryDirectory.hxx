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

#ifndef REXSAPI_TEST_TEMPORARY_DIRECTOTY_HXX
#define REXSAPI_TEST_TEMPORARY_DIRECTOTY_HXX

#include <rexsapi/Exception.hxx>

#include <filesystem>

#ifdef WIN32
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <windows.h>
#else
  #ifdef __linux__
    #include <cstring>
    #include <linux/limits.h>
  #endif
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <unistd.h>
#endif


class TemporaryDirectory
{
public:
  TemporaryDirectory()
  : m_Path{getInternalTempDirectoryPath()}
  {
  }

  const std::filesystem::path& getTempDirectoryPath() const
  {
    return m_Path;
  }

  ~TemporaryDirectory()
  {
    std::error_code ec;
    std::filesystem::remove_all(m_Path, ec);
  }

  TemporaryDirectory(const TemporaryDirectory&) = delete;
  TemporaryDirectory(TemporaryDirectory&&) = delete;
  TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;
  TemporaryDirectory& operator=(TemporaryDirectory&&) = delete;

private:
  static std::filesystem::path getInternalTempDirectoryPath()
  {
    std::filesystem::path tempDirectory{std::filesystem::temp_directory_path()};

#ifdef WIN32
    WCHAR tmpDir[MAX_PATH];
    if (GetTempFileNameW(tempDirectory.c_str(), L"rexsapi", 0, tmpDir) == 0) {
      throw rexsapi::TException{fmt::format("cannot create temp directory: {}", GetLastError())};
    }
    std::filesystem::remove(tmpDir);
    std::filesystem::create_directories(tmpDir);
#else
    char tmp[PATH_MAX];
    ::strncpy(tmp, (tempDirectory / "rexsapi_XXXXXX").c_str(), PATH_MAX - 1);

    const char* tmpDir = ::mkdtemp(tmp);
    if (tmpDir == nullptr) {
      throw rexsapi::TException{"cannot create temp directory"};
    }
#endif

    return std::filesystem::path{tmpDir};
  }

  std::filesystem::path m_Path;
};

#endif
