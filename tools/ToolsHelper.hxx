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

#include <rexsapi/FileTypes.hxx>

#include <iostream>
#include <regex>
#include <set>

inline static void processDirectory(bool recurse, const std::filesystem::path& path, std::set<std::filesystem::path>& models)
{
  if (!std::filesystem::is_directory(path)) {
    return;
  }

  for (const auto& entry : std::filesystem::directory_iterator{path}) {
    if (std::filesystem::is_regular_file(entry.path()) &&
        rexsapi::TExtensionChecker{}.getFileType(entry.path()) != rexsapi::TFileType::UNKNOWN) {
      models.emplace(entry.path());
    } else if (recurse && std::filesystem::is_directory(entry.path())) {
      processDirectory(true, entry.path(), models);
    }
  }
}


inline static std::vector<std::filesystem::path> getModels(bool recurse, const std::vector<std::filesystem::path>& paths)
{
  std::set<std::filesystem::path> models;

  for (const auto& path : paths) {
    if (std::filesystem::is_regular_file(path)) {
      models.emplace(path);
    } else {
      processDirectory(recurse, path, models);
    }
  }
  return std::vector<std::filesystem::path>{models.begin(), models.end()};
}


inline static rexsapi::TCustomExtensionMappings getCustomMappings(const std::vector<std::string>& mappings)
{
  rexsapi::TCustomExtensionMappings customMappings;
  static std::regex regExpr(R"(^([^\s:]+):(.+)$)");

  for (const auto& mapping : mappings) {
    std::smatch match;
    if (std::regex_search(mapping, match, regExpr)) {
      customMappings.emplace_back(rexsapi::TCustomExtensionMapping{match[1], rexsapi::fileTypeFromString(match[2])});
    } else {
      throw std::runtime_error{fmt::format("not a valid custom extension mapping: '{}'", mapping)};
    }
  }

  return customMappings;
}
