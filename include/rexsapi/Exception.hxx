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

#ifndef REXSAPI_EXCEPTION_HXX
#define REXSAPI_EXCEPTION_HXX

#include <stdexcept>

namespace rexsapi
{
  /**
   * @brief Exception class for all REXS exceptions.
   *
   * There is only one exception type and no exception hierarchie.
   */
  class TException : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };
}

#endif
