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

#include <rexsapi/ZipArchive.hxx>

#include <test/TestHelper.hxx>

#include <doctest.h>

TEST_CASE("Zip archive test")
{
  rexsapi::TExtensionChecker extensionChecker;

  SUBCASE("Load existing zip")
  {
    rexsapi::detail::ZipArchive archive{projectDir() / "test" / "example_models" / "example_json.rexs.zip",
                                        extensionChecker};
    const auto [value, type] = archive.load();
    CHECK(type == rexsapi::TFileType::JSON);
    CHECK(value.size());
  }

  SUBCASE("Load zip with no rexs file")
  {
    CHECK_THROWS(
      rexsapi::detail::ZipArchive{projectDir() / "test" / "example_models" / "no_rexs_file.rexsz", extensionChecker});
  }

  SUBCASE("Load non-existing zip")
  {
    CHECK_THROWS(
      rexsapi::detail::ZipArchive{projectDir() / "test" / "example_models" / "does_not_exist.rexsz", extensionChecker});
  }
}
