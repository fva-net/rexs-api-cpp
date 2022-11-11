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

#ifndef REXSAPI_BASE64_HXX
#define REXSAPI_BASE64_HXX

#include <rexsapi/Exception.hxx>

#include <array>
#include <string>
#include <string_view>
#include <vector>


/*
  Based on public domain code from wikibooks
  https://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64
*/

namespace rexsapi::detail
{
  static inline std::string base64Encode(const uint8_t* data, const size_t len)
  {
    static constexpr const char* base64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result((len + 2) / 3 * 4, '=');
    char* str = result.data();
    size_t resultIndex = 0;
    size_t padCount = len % 3;
    uint32_t n = 0;

    for (size_t x = 0; x < len; x += 3) {
      n = ((uint32_t)data[x]) << 16;

      if ((x + 1) < len)
        n += ((uint32_t)data[x + 1]) << 8;

      if ((x + 2) < len)
        n += data[x + 2];

      str[resultIndex++] = base64chars[(uint8_t)(n >> 18) & 63];
      str[resultIndex++] = base64chars[(uint8_t)(n >> 12) & 63];

      if ((x + 1) < len) {
        str[resultIndex++] = base64chars[(uint8_t)(n >> 6) & 63];
      }

      if ((x + 2) < len) {
        str[resultIndex++] = base64chars[(uint8_t)n & 63];
      }
    }

    if (padCount > 0) {
      for (; padCount < 3; padCount++) {
        str[resultIndex++] = '=';
      }
    }

    return result;
  }


  static inline std::vector<uint8_t> base64Decode(std::string_view data)
  {
    static constexpr const std::array<uint8_t, 256> d = {
      66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 64, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
      66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 62, 66, 66, 66, 63, 52, 53, 54, 55,
      56, 57, 58, 59, 60, 61, 66, 66, 66, 65, 66, 66, 66, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
      13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 66, 66, 66, 66, 66, 66, 26, 27, 28, 29, 30, 31, 32,
      33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 66, 66, 66, 66, 66, 66, 66,
      66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
      66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
      66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
      66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
      66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66};

    static constexpr uint8_t WHITESPACE = 64;
    static constexpr uint8_t EQUALS = 65;
    static constexpr uint8_t INVALID = 66;

    std::vector<uint8_t> result;
    const auto len = data.size();
    if (len == 0) {
      return result;
    }

    const auto* p = reinterpret_cast<const uint8_t*>(data.data());
    size_t j = 0;
    const size_t pad1 = len % 4 || p[len - 1] == '=';
    const size_t pad2 = pad1 && (len % 4 > 2 || p[len - 2] != '=');
    const size_t last = (len - pad1) / 4 << 2;
    result.resize(last / 4 * 3 + pad1 + pad2);

    char iter = 0;
    uint32_t buf = 0;

    for (const auto cc : data) {
      uint8_t c = d[static_cast<uint8_t>(cc)];

      switch (c) {
        case WHITESPACE:
          continue;
        case INVALID:
          throw TException{"cannot decode base64 string: invalid data"};
        case EQUALS: /* pad character, end of data */
          break;
        default:
          buf = buf << 6 | c;
          iter++;
          if (iter == 4) {
            result[j++] = (buf >> 16) & 255;
            result[j++] = (buf >> 8) & 255;
            result[j++] = buf & 255;
            buf = 0;
            iter = 0;
          }
      }
    }

    if (iter == 3) {
      result[j++] = (buf >> 10) & 255;
      result[j++] = (buf >> 2) & 255;
    } else if (iter == 2) {
      result[j++] = (buf >> 4) & 255;
    }

    return result;
  }
}

#endif
