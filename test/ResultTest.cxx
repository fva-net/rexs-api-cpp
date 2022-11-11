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

#include <rexsapi/Result.hxx>

#include <doctest.h>


TEST_CASE("Loader result test")
{
  SUBCASE("No errors")
  {
    rexsapi::TResult result{};
    CHECK(result);
    CHECK_FALSE(result.hasIssues());
  }

  SUBCASE("With errors")
  {
    rexsapi::TResult result{};
    result.addError(rexsapi::TError{rexsapi::TErrorLevel::ERR, "my first message"});
    result.addError(rexsapi::TError{rexsapi::TErrorLevel::ERR, "my second message", 32});

    CHECK_FALSE(result);
    CHECK(result.hasIssues());
    CHECK_FALSE(result.isCritical());
    REQUIRE(result.getErrors().size() == 2);
    CHECK(result.getErrors()[0].getMessage() == "my first message");
    CHECK(result.getErrors()[0].isError());
    CHECK(result.getErrors()[1].getMessage() == "my second message: offset 32");
    CHECK(result.getErrors()[1].isError());

    result.reset();
    CHECK(result);
    CHECK_FALSE(result.isCritical());
  }

  SUBCASE("With errors and critical")
  {
    rexsapi::TResult result{};
    result.addError(rexsapi::TError{rexsapi::TErrorLevel::ERR, "my first message"});
    result.addError(rexsapi::TError{rexsapi::TErrorLevel::CRIT, "my second message", 32});

    CHECK_FALSE(result);
    CHECK(result.hasIssues());
    CHECK(result.isCritical());
    REQUIRE(result.getErrors().size() == 2);
    CHECK(result.getErrors()[0].getMessage() == "my first message");
    CHECK(result.getErrors()[0].isError());
    CHECK_FALSE(result.getErrors()[0].isCritical());
    CHECK(result.getErrors()[1].getMessage() == "my second message: offset 32");
    CHECK(result.getErrors()[1].isError());
    CHECK(result.getErrors()[1].isCritical());

    result.reset();
    CHECK(result);
    CHECK_FALSE(result.isCritical());
  }

  SUBCASE("With warnings")
  {
    rexsapi::TResult result{};
    result.addError(rexsapi::TError{rexsapi::TErrorLevel::WARN, "my first message"});
    result.addError(rexsapi::TError{rexsapi::TErrorLevel::WARN, "my second message", 32});

    CHECK(result);
    CHECK(result.hasIssues());
    CHECK_FALSE(result.isCritical());
    REQUIRE(result.getErrors().size() == 2);
    CHECK(result.getErrors()[0].getMessage() == "my first message");
    CHECK(result.getErrors()[0].isWarning());
    CHECK(result.getErrors()[1].getMessage() == "my second message: offset 32");
    CHECK(result.getErrors()[1].isWarning());

    result.reset();
    CHECK(result);
    CHECK_FALSE(result.isCritical());
  }

  SUBCASE("Error level to string")
  {
    CHECK(rexsapi::toErrorLevelString(rexsapi::TErrorLevel::WARN) == "WARNING");
    CHECK(rexsapi::toErrorLevelString(rexsapi::TErrorLevel::ERR) == "ERROR");
    CHECK(rexsapi::toErrorLevelString(rexsapi::TErrorLevel::CRIT) == "CRITICAL");
  }
}
