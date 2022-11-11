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

#ifndef REXSAPI_FILE_UTILS_HXX
#define REXSAPI_FILE_UTILS_HXX

#include <rexsapi/Result.hxx>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace rexsapi::detail
{
  static inline std::vector<uint8_t> loadFile(TResult& result, const std::filesystem::path& path)
  {
    if (!std::filesystem::exists(path)) {
      result.addError(TError{TErrorLevel::CRIT, fmt::format("'{}' does not exist", path.string())});
      return {};
    }
    if (!std::filesystem::is_regular_file(path)) {
      result.addError(TError{TErrorLevel::CRIT, fmt::format("'{}' is not a regular file", path.string())});
      return {};
    }
    std::ifstream file{path};
    if (!file.good()) {
      result.addError(TError{TErrorLevel::CRIT, fmt::format("'{}' cannot be loaded", path.string())});
      return {};
    }

    std::stringstream stream;
    stream << file.rdbuf();
    auto buffer = stream.str();
    if (!file.good() || buffer.empty()) {
      result.addError(TError{TErrorLevel::CRIT, fmt::format("'{}' cannot be loaded", path.string())});
      return {};
    }
    return std::vector<uint8_t>{buffer.begin(), buffer.end()};
  }
}

#endif
