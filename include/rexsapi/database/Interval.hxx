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

#ifndef REXSAPI_DATABASE_INTERVAL_HXX
#define REXSAPI_DATABASE_INTERVAL_HXX

/** @file */

namespace rexsapi::database
{
  enum class TIntervalType {
    OPEN,   //!< Represents an open interval
    CLOSED  //!< Represents a closed interval
  };


  /**
   * @brief Represents a specific endpoint of a TInterval.
   *
   * Contains the limit and the nature of one interval endpoint, either open or closed
   */
  class TIntervalEndpoint
  {
  public:
    TIntervalEndpoint() = default;

    TIntervalEndpoint(double limit, TIntervalType type)
    : m_Set{true}
    , m_Type{type}
    , m_Limit{limit}
    {
    }

    /**
     * @brief Checks if this interval endpoint has been set.
     *
     * @return true if the endpoint has been set
     * @return false if the endpoint has been default constructed
     */
    [[nodiscard]] bool isSet() const noexcept
    {
      return m_Set;
    }

    /**
     * @brief Checks if the value is smaller or equal to the limit.
     *
     * The result will depend on the nature of the endpoint: either open or closed.
     * A closed endpoint will include the limit while an open will not.
     *
     * @param value The value to check
     * @return true if the value is smaller or equal to the limit in case of a closed endpoint.
     * @return true if the value is smaller than the limit in case of an open endpoint.
     * @return false if the value is outside of the limit
     */
    bool operator<=(double value) const noexcept;

    /**
     * @brief Checks if the value is greater or equal to the limit.
     *
     * The result will depend on the nature of the endpoint: either open or closed.
     * A closed endpoint will include the limit while an open will not.
     *
     * @param value The value to check
     * @return true if the value is greater or equal to the limit in case of a closed endpoint.
     * @return true if the value is greater than the limit in case of an open endpoint.
     * @return false if the value is outside of the limit
     */
    bool operator>=(double value) const noexcept;

  private:
    bool m_Set{false};
    TIntervalType m_Type{TIntervalType::OPEN};
    double m_Limit{0.0};
  };


  /**
   * @brief Represents a value range for an attributes value.
   *
   * Intervals should not be created manually, but will be imported from the REXS database model using the
   * TModelRegistry.
   *
   * TInterval objects correspond to value ranges loaded alongside their attributes from a specific REXS
   * database model version.
   */
  class TInterval
  {
  public:
    /**
     * @brief Constructs a new TInterval object with no interval set.
     *
     * Intervals are immutable objects, once created they cannot be changed.
     *
     * All checks of a default TInterval will return true
     */
    TInterval() = default;

    /**
     * @brief Constructs a new TInterval object.
     *
     * Intervals are immutable objects, once created they cannot be changed.
     *
     * @param min The minimum limit for the interval
     * @param max The maximum limit for the interval
     */
    TInterval(TIntervalEndpoint min, TIntervalEndpoint max)
    : m_Min{min}
    , m_Max{max}
    {
    }

    /**
     * @brief Checks the value against the interval range.
     *
     * @param value The value to check
     * @return true if the value lies in the interval
     * @return false if the value lies outside the interval
     */
    [[nodiscard]] bool check(double value) const noexcept
    {
      return m_Min <= value && m_Max >= value;
    }

  private:
    TIntervalEndpoint m_Min{};
    TIntervalEndpoint m_Max{};
  };


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  inline bool TIntervalEndpoint::operator<=(double value) const noexcept
  {
    if (!m_Set) {
      return true;
    }

    switch (m_Type) {
      case TIntervalType::OPEN:
        return m_Limit < value;
      case TIntervalType::CLOSED:
        return m_Limit <= value;
    }

    return false;
  }

  inline bool TIntervalEndpoint::operator>=(double value) const noexcept
  {
    if (!m_Set) {
      return true;
    }

    switch (m_Type) {
      case TIntervalType::OPEN:
        return m_Limit > value;
      case TIntervalType::CLOSED:
        return m_Limit >= value;
    }

    return false;
  }

}

#endif
