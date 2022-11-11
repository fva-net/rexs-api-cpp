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

#include <rexsapi/database/Model.hxx>

#include <doctest.h>


TEST_CASE("Status test")
{
  SUBCASE("Existing status from string")
  {
    CHECK(rexsapi::database::statusFromString("RELEASED") == rexsapi::database::TStatus::RELEASED);
    CHECK(rexsapi::database::statusFromString("IN_DEVELOPMENT") == rexsapi::database::TStatus::IN_DEVELOPMENT);
  }

  SUBCASE("Non existing status from string")
  {
    CHECK_THROWS_WITH(rexsapi::database::statusFromString("PUSCHEL"), "status 'PUSCHEL' unkown");
  }
}

TEST_CASE("Database Model test")
{
  SUBCASE("Create model")
  {
    rexsapi::database::TModel model(rexsapi::TRexsVersion{"1.4"}, "de", "2022-04-20T14:18:11.344+02:00",
                                    rexsapi::database::TStatus::RELEASED);
    CHECK(model.getVersion() == rexsapi::TRexsVersion{"1.4"});
    CHECK(model.getLanguage() == "de");
    CHECK(model.getDate() == "2022-04-20T14:18:11.344+02:00");
    CHECK(model.isReleased());
  }

  SUBCASE("Not released model")
  {
    rexsapi::database::TModel model(rexsapi::TRexsVersion{"1.4"}, "de", "2022-04-20T14:18:11.344+02:00",
                                    rexsapi::database::TStatus::IN_DEVELOPMENT);
    CHECK_FALSE(model.isReleased());
  }

  SUBCASE("non existing component")
  {
    rexsapi::database::TModel model(rexsapi::TRexsVersion{"1.4"}, "de", "2022-04-20T14:18:11.344+02:00",
                                    rexsapi::database::TStatus::RELEASED);
    CHECK_THROWS_WITH((void)model.findComponentById("hutzli"), "component 'hutzli' not found in database");
  }
}
