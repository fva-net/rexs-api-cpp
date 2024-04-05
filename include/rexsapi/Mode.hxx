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

#ifndef REXSAPI_MODE_HXX
#define REXSAPI_MODE_HXX

#include <rexsapi/Exception.hxx>
#include <rexsapi/Result.hxx>

namespace rexsapi
{
  /**
   * @brief Defines how to handle issues found while processing models.
   *
   * In RELAXED_MODE most errors will be downgraded to warnings to allow processing of non standard compliant models.
   * The STRICT_MODE will leave all errors as they are.
   */
  enum class TMode {
    STRICT_MODE,  //!< Don't change errors
    RELAXED_MODE  //!< Downgrade errors to warnings
  };

  /**
   * @brief Returns the string representation of a mode.
   *
   * @param mode The mode to convert
   * @return std::string representation of the mode
   * @throws TException if an unknown mode was supplied
   */
  static std::string toModeString(TMode mode);


  namespace detail
  {
    class TModeAdapter
    {
    public:
      explicit TModeAdapter(TMode mode)
      : m_Mode{mode}
      {
      }

      TErrorLevel adapt(TErrorLevel level) const noexcept;

      TMode getMode() const
      {
        return m_Mode;
      }

    private:
      TMode m_Mode;
    };
  }

  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  static inline std::string toModeString(TMode mode)
  {
    switch (mode) {
      case TMode::STRICT_MODE:
        return "strict";
      case TMode::RELAXED_MODE:
        return "relaxed";
    }
    throw TException{"unknown mode type"};
  }

  inline TErrorLevel detail::TModeAdapter::adapt(TErrorLevel level) const noexcept
  {
    switch (m_Mode) {
      case TMode::RELAXED_MODE:
        switch (level) {
          case TErrorLevel::CRIT:
            return TErrorLevel::CRIT;
          case TErrorLevel::WARN:
            [[fallthrough]];
          case TErrorLevel::ERR:
            return TErrorLevel::WARN;
        }
        break;
      case TMode::STRICT_MODE:
        return level;
    }
    return level;
  }

}

#endif
