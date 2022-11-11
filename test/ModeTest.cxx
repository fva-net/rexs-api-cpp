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

#include <rexsapi/Mode.hxx>

#include <doctest.h>


TEST_CASE("Mode test")
{
  SUBCASE("To string")
  {
    CHECK(rexsapi::toModeString(rexsapi::TMode::STRICT_MODE) == "strict");
    CHECK(rexsapi::toModeString(rexsapi::TMode::RELAXED_MODE) == "relaxed");
    CHECK_THROWS(rexsapi::toModeString(static_cast<rexsapi::TMode>(27)));
  }

  SUBCASE("Strict mode adaptor")
  {
    rexsapi::detail::TModeAdapter adapter{rexsapi::TMode::STRICT_MODE};

    CHECK(adapter.adapt(rexsapi::TErrorLevel::CRIT) == rexsapi::TErrorLevel::CRIT);
    CHECK(adapter.adapt(rexsapi::TErrorLevel::ERR) == rexsapi::TErrorLevel::ERR);
    CHECK(adapter.adapt(rexsapi::TErrorLevel::WARN) == rexsapi::TErrorLevel::WARN);
  }

  SUBCASE("Relaxed mode adaptor")
  {
    rexsapi::detail::TModeAdapter adapter{rexsapi::TMode::RELAXED_MODE};

    CHECK(adapter.adapt(rexsapi::TErrorLevel::CRIT) == rexsapi::TErrorLevel::CRIT);
    CHECK(adapter.adapt(rexsapi::TErrorLevel::ERR) == rexsapi::TErrorLevel::WARN);
    CHECK(adapter.adapt(rexsapi::TErrorLevel::WARN) == rexsapi::TErrorLevel::WARN);
  }
}
