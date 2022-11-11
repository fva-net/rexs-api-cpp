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

#include <rexsapi/RexsVersion.hxx>

#include <doctest.h>

TEST_CASE("Rexs version test")
{
  SUBCASE("Create from string")
  {
    rexsapi::TRexsVersion version{"1.4"};
    CHECK(version.getMajor() == 1);
    CHECK(version.getMinor() == 4);
  }

  SUBCASE("Create from string fail")
  {
    CHECK_THROWS(rexsapi::TRexsVersion{"14"});
    CHECK_THROWS(rexsapi::TRexsVersion{" 1.4"});
    CHECK_THROWS(rexsapi::TRexsVersion{"hutzli"});
  }

  SUBCASE("Create from integer")
  {
    rexsapi::TRexsVersion version{1, 4};
    CHECK(version.getMajor() == 1);
    CHECK(version.getMinor() == 4);
  }

  SUBCASE("To string")
  {
    CHECK(rexsapi::TRexsVersion{"1.4"}.asString() == "1.4");
    CHECK(rexsapi::TRexsVersion{"21.99"}.asString() == "21.99");
  }

  SUBCASE("Compare")
  {
    CHECK(rexsapi::TRexsVersion{"1.4"} == rexsapi::TRexsVersion{"1.4"});
    CHECK(rexsapi::TRexsVersion{"1.4"} != rexsapi::TRexsVersion{"1.3"});
    CHECK_FALSE(rexsapi::TRexsVersion{"1.4"} != rexsapi::TRexsVersion{"1.4"});
    CHECK_FALSE(rexsapi::TRexsVersion{"1.4"} == rexsapi::TRexsVersion{"1.3"});

    CHECK(rexsapi::TRexsVersion{"1.4"} > rexsapi::TRexsVersion{"1.3"});
    CHECK(rexsapi::TRexsVersion{"1.3"} < rexsapi::TRexsVersion{"1.4"});
    CHECK(rexsapi::TRexsVersion{"2.1"} > rexsapi::TRexsVersion{"1.9"});
    CHECK(rexsapi::TRexsVersion{"1.9"} < rexsapi::TRexsVersion{"2.0"});

    CHECK_FALSE(rexsapi::TRexsVersion{"1.4"} < rexsapi::TRexsVersion{"1.4"});
    CHECK_FALSE(rexsapi::TRexsVersion{"1.4"} > rexsapi::TRexsVersion{"1.4"});

    CHECK(rexsapi::TRexsVersion{"1.4"} <= rexsapi::TRexsVersion{"1.4"});
    CHECK(rexsapi::TRexsVersion{"1.4"} >= rexsapi::TRexsVersion{"1.4"});
    CHECK(rexsapi::TRexsVersion{"1.4"} <= rexsapi::TRexsVersion{"2.4"});
    CHECK(rexsapi::TRexsVersion{"2.0"} >= rexsapi::TRexsVersion{"1.4"});

    CHECK_FALSE(rexsapi::TRexsVersion{"1.0"} >= rexsapi::TRexsVersion{"1.4"});
    CHECK_FALSE(rexsapi::TRexsVersion{"17.0"} <= rexsapi::TRexsVersion{"1.4"});
  }
}
