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

#include <rexsapi/Base64.hxx>

#include <doctest.h>


namespace
{
  void checkBase64(std::string_view input, std::string_view output)
  {
    const auto encoded = rexsapi::detail::base64Encode(reinterpret_cast<const uint8_t*>(input.data()), input.size());
    CHECK(output.compare(encoded) == 0);
    const auto decoded = rexsapi::detail::base64Decode(output);
    CHECK(input.compare(std::string_view{reinterpret_cast<const char*>(decoded.data()), decoded.size()}) == 0);
  }
}

TEST_CASE("Base64 test")
{
  SUBCASE("Encoding / decoding")
  {
    checkBase64("", "");
    checkBase64("f", "Zg==");
    checkBase64("foo", "Zm9v");
    checkBase64("foob", "Zm9vYg==");
    checkBase64("fooba", "Zm9vYmE=");
    checkBase64("foobar", "Zm9vYmFy");
  }
}
