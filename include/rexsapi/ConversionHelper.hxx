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

#ifndef REXSAPI_CONVERSION_HELPER_HXX
#define REXSAPI_CONVERSION_HELPER_HXX

#include <rexsapi/Exception.hxx>
#include <rexsapi/Format.hxx>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace rexsapi
{
  /**
   * @brief Converts a string into an unsigned 64 bit integer.
   *
   * The conversion will only be successful, if the string actually contains an unsigned 64 bit integer and is in range.
   *
   * @param s The string to convert.
   * @return uint64_t converted from the string
   * @throws TException if the string is not an integer or out of range. Will also throw if the string contains other
   * characters after the integer.
   */
  static inline uint64_t convertToUint64(const std::string& s)
  {
    if (s.find_first_of('-') == std::string::npos) {
      try {
        size_t pos = 0;
        auto val = std::stoull(s, &pos);
        if (pos == s.length()) {
          return val;
        }
      } catch (const std::invalid_argument&) {
        throw TException{fmt::format("cannot convert string '{}' to unsigned integer: invalid argument", s)};
      } catch (const std::out_of_range&) {
        throw TException{fmt::format("cannot convert string '{}' to unsigned integer: out of range", s)};
      }
    }

    throw TException{fmt::format("cannot convert string to unsigned integer: {}", s)};
  }


  /**
   * @brief Converts a string into a signed 64 bit integer.
   *
   * The conversion will only be successful, if the string actually contains a signed 64 bit integer and is in range.
   *
   * @param s The string to convert.
   * @return int64_t converted from the string
   * @throws TException if the string is not an integer or out of range. Will also throw if the string contains other
   * characters after the integer.
   */
  static inline int64_t convertToInt64(const std::string& s)
  {
    try {
      size_t pos = 0;
      auto val = std::stoll(s, &pos);
      if (pos == s.length()) {
        return val;
      }
    } catch (const std::invalid_argument&) {
      throw TException{fmt::format("cannot convert string '{}' to integer: invalid argument", s)};
    } catch (const std::out_of_range&) {
      throw TException{fmt::format("cannot convert string '{}' to integer: out of range", s)};
    }

    throw TException{fmt::format("cannot convert string to integer: {}", s)};
  }


  /**
   * @brief Converts a string into a double.
   *
   * The conversion will only be successful, if the string actually contains a double and is in range.
   *
   * @param s The string to convert.
   * @return double converted from the string
   * @throws TException if the string is not a double or out of range. Will also throw if the string contains other
   * characters after the integer.
   */
  static inline double convertToDouble(const std::string& s)
  {
    try {
      std::size_t pos = 0;
      auto val = std::stod(s, &pos);
      if (pos == s.length()) {
        return val;
      }
    } catch (const std::invalid_argument&) {
      throw TException{fmt::format("cannot convert string '{}' to double: invalid argument", s)};
    } catch (const std::out_of_range&) {
      throw TException{fmt::format("cannot convert string '{}' to double: out of range", s)};
    }

    throw TException{fmt::format("cannot convert string to double: {}", s)};
  }

  /**
   * @brief Formats a double according to the REXS standard.
   *
   * The string will have a maximum of 15 significant digits. Will output scientific format if appropriate.
   *
   * @param d The double to format
   * @return std::string of the conversion
   */
  static inline std::string format(double d)
  {
    auto s = fmt::format("{:0.15G}", d);
    if (s.find_last_of('.') == std::string::npos && s.find('E') == std::string::npos) {
      s += ".0";
    }
    return s;
  }

  /**
   * @brief Returns an ISO8601 string representation of the given time point.
   *
   * @param timepoint The timepoint to format
   * @return std::string of the conversion
   */
  static inline std::string getTimeStringISO8601(std::chrono::system_clock::time_point timepoint)
  {
    const std::time_t t_c = std::chrono::system_clock::to_time_t(timepoint);
    tm buf{};
    std::ostringstream stream;
#if defined(WIN32)
    localtime_s(&buf, &t_c);
    stream << std::put_time(&buf, "%FT%T%z");
#else
    stream << std::put_time(localtime_r(&t_c, &buf), "%FT%T%z");
#endif
    std::string s{stream.str()};
    return s.insert(22, ":");
  }

  /**
   * @brief Converts a string to upper case.
   *
   * @param s String to convert
   * @return std::string The converted string
   */
  static inline std::string toupper(std::string s)
  {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
      return static_cast<char>(std::toupper(c));
    });
    return s;
  }
}

#endif
