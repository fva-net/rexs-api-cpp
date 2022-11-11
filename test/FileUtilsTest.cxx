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

#include <rexsapi/FileUtils.hxx>

#include <test/TemporaryDirectory.hxx>
#include <test/TestHelper.hxx>

#include <doctest.h>

TEST_CASE("File utils test")
{
  rexsapi::TResult result;
  TemporaryDirectory tmpDir;

  SUBCASE("Load existing file")
  {
    auto buffer = rexsapi::detail::loadFile(result, projectDir() / "models" / "rexs-schema.json");
    CHECK(result);
    CHECK_FALSE(buffer.empty());
  }

  SUBCASE("Load non-existing file")
  {
    auto buffer = rexsapi::detail::loadFile(result, tmpDir.getTempDirectoryPath() / "puschel.txt");
    CHECK_FALSE(result);
    CHECK(buffer.empty());
  }

  SUBCASE("Load directory as file")
  {
    auto buffer = rexsapi::detail::loadFile(result, tmpDir.getTempDirectoryPath());
    CHECK_FALSE(result);
    CHECK(buffer.empty());
  }

  SUBCASE("Load file with no permission")
  {
    auto path = tmpDir.getTempDirectoryPath() / "test.txt";
    std::ofstream stream{path};
    stream.close();

    std::filesystem::permissions(
      path, std::filesystem::perms::owner_all | std::filesystem::perms::group_all | std::filesystem::perms::others_all,
      std::filesystem::perm_options::remove);

    auto buffer = rexsapi::detail::loadFile(result, path);
    CHECK_FALSE(result);
    CHECK(buffer.empty());
  }
}
