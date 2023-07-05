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

#ifndef REXSAPI_RESOURCE_LOADER_HXX
#define REXSAPI_RESOURCE_LOADER_HXX

#include <rexsapi/Defines.hxx>
#include <rexsapi/Exception.hxx>
#include <rexsapi/Format.hxx>

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace rexsapi
{
  /**
   * @brief Defines the severity of an error.
   *
   */
  enum class TErrorLevel {
    WARN,  //!< A warning. Something is not ok, but can be processed without concern.
    ERR,   //!< An error. Processing may go on, but elements might be missing or are broken.
    CRIT   //!< A critical error. Processing cannot go on.
  };

  /**
   * @brief Returns the string representation of an error level.
   *
   * @param level The error level to get the string representation for
   * @return std::string representation of the error level
   */
  static inline std::string toErrorLevelString(TErrorLevel level)
  {
    switch (level) {
      case TErrorLevel::WARN:
        return "WARNING";
      case TErrorLevel::ERR:
        return "ERROR";
      case TErrorLevel::CRIT:
        return "CRITICAL";
    }
    throw TException{"unknown error level"};
  }


  /**
   * @brief Represents an error.
   *
   * Errors are created for most of the problemes processing models. They all contain a severity and a message. They
   * might contain a position, if the error occured while processing files.
   *
   */
  class TError
  {
  public:
    explicit TError(TErrorLevel level, std::string message, ssize_t position = -1)
    : m_Level{level}
    , m_Message{std::move(message)}
    , m_Position{position}
    {
    }

    /**
     * @brief Returns if this error is of error or critical level.
     *
     * @return true either error or critical level
     * @return false a warning
     */
    bool isError() const noexcept
    {
      return m_Level == TErrorLevel::ERR || m_Level == TErrorLevel::CRIT;
    }

    /**
     * @brief Returns if this error is of critical level.
     *
     * @return true critical level
     * @return false either error or warning level
     */
    bool isCritical() const noexcept
    {
      return m_Level == TErrorLevel::CRIT;
    }

    /**
     * @brief Returns if this error is of warning level.
     *
     * @return true warning level
     * @return false either error or critical level
     */
    bool isWarning() const noexcept
    {
      return m_Level == TErrorLevel::WARN;
    }

    std::string getMessage() const
    {
      if (m_Position != -1) {
        return fmt::format("{}: offset {}", m_Message, m_Position);
      }
      return fmt::format("{}", m_Message);
    }

  private:
    TErrorLevel m_Level;
    std::string m_Message;
    ssize_t m_Position;
  };


  /**
   * @brief Represents the outcome of some operation.
   *
   * Many operation either return or take a result as argument. A result can contain any number of errors, that describe
   * and categorize the issues discovered while processing. The result has a bool operator and can be converted to true
   * or false. True will always mean, that there is at least an error or critical contained in the result. Warnings will
   * always convert to false.
   *
   */
  class TResult
  {
  public:
    void addError(TError error) noexcept
    {
      m_Errors.emplace_back(std::move(error));
    }

    /**
     * @brief Returns the status of the result.
     *
     * @return true there are errors with error or critical level
     * @return false there is either no error or only warnings
     */
    explicit operator bool() const noexcept
    {
      return m_Errors.empty() || std::find_if(m_Errors.begin(), m_Errors.end(), [](const auto& error) {
                                   return error.isError();
                                 }) == m_Errors.end();
    }

    /**
     * @brief Returns if the result contains critical errors.
     *
     * @return true there are critical errors
     * @return false no critcal errors
     */
    bool isCritical() const noexcept
    {
      return !m_Errors.empty() && std::find_if(m_Errors.begin(), m_Errors.end(), [](const auto& error) {
                                    return error.isCritical();
                                  }) != m_Errors.end();
    }

    bool hasIssues() const noexcept
    {
      return !m_Errors.empty();
    }

    const std::vector<TError>& getErrors() const& noexcept
    {
      return m_Errors;
    }

    /**
     * @brief Removes all errors.
     *
     * The result can be reused after this operation.
     *
     */
    void reset() noexcept
    {
      m_Errors.clear();
    }

  private:
    std::vector<TError> m_Errors;
  };
}

#endif
