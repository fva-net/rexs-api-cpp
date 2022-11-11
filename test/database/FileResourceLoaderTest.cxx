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

#include <rexsapi/database/FileResourceLoader.hxx>

#include <test/TemporaryDirectory.hxx>
#include <test/TestHelper.hxx>

#include <doctest.h>

namespace
{
  void checkBuffer(const std::vector<uint8_t>& buffer)
  {
    std::string v{buffer.begin(), buffer.end()};
    CHECK(v.find("<?xml") != std::string::npos);
    CHECK(v.find("<rexsModel") != std::string::npos);
    CHECK(v.find("</rexsModel>") != std::string::npos);
  }
}

TEST_CASE("File resource loader test")
{
  SUBCASE("Load existing resources")
  {
    rexsapi::database::TFileResourceLoader loader{projectDir() / "models"};

    std::vector<std::vector<uint8_t>> buffers;
    loader.load([&buffers](const rexsapi::TResult&, std::vector<uint8_t>& buffer) {
      buffers.emplace_back(buffer);
    });

    CHECK(buffers.size() == 10);
    std::for_each(buffers.begin(), buffers.end(), [](const auto& buf) {
      checkBuffer(buf);
    });
  }

  SUBCASE("Load not existing path")
  {
    rexsapi::database::TFileResourceLoader loader{projectDir() / "non-existing-models"};
    CHECK_THROWS(loader.load([](const rexsapi::TResult&, std::vector<uint8_t>&) {
      // nothing to do
    }));
  }

  SUBCASE("No callback")
  {
    rexsapi::database::TFileResourceLoader loader{projectDir() / "models"};
    CHECK_THROWS(loader.load({}));
  }

  SUBCASE("No files")
  {
    TemporaryDirectory guard{};
    rexsapi::database::TFileResourceLoader loader{guard.getTempDirectoryPath()};
    auto result = loader.load([](const rexsapi::TResult&, std::vector<uint8_t>&) {
      // nothing to do
    });
    CHECK_FALSE(result);
  }
}
