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

#include <rexsapi/RelationTypeChecker.hxx>

#include <test/TestModelLoader.hxx>

#include <doctest.h>

TEST_CASE("Relation type checker test")
{
  const auto& dbModel = loadModel("1.4");

  rexsapi::TRelationTypeChecker checker{rexsapi::TMode::STRICT_MODE};
  rexsapi::TResult result;
  rexsapi::TComponent comonent1{1, dbModel.findComponentById("planetary_stage"), "", {}};
  rexsapi::TComponent comonent2{2, dbModel.findComponentById("shaft"), "", {}};

  SUBCASE("Load mappings")
  {
    auto mappings = rexsapi::detail::loadMappings();
    CHECK(mappings.size() == 4);
  }

  SUBCASE("Check relation")
  {
    rexsapi::TRelationReferences references;
    references.emplace_back(rexsapi::TRelationReference{rexsapi::TRelationRole::PLANETARY_STAGE, "", comonent1});
    references.emplace_back(rexsapi::TRelationReference{rexsapi::TRelationRole::SHAFT, "", comonent2});
    rexsapi::TRelation relation{rexsapi::TRelationType::PLANET_SHAFT, {}, references};

    CHECK(checker.check(result, rexsapi::TRexsVersion{1, 4}, relation));
    CHECK(result);
    CHECK_FALSE(result.hasIssues());
  }

  SUBCASE("Check relation missing roles")
  {
    rexsapi::TRelationReferences references;
    rexsapi::TRelation relation{rexsapi::TRelationType::PLANET_SHAFT, {}, references};

    CHECK_FALSE(checker.check(result, rexsapi::TRexsVersion{1, 4}, relation));
    CHECK_FALSE(result);
    CHECK(result.hasIssues());
  }

  SUBCASE("Unknown version")
  {
    rexsapi::TRelation relation{rexsapi::TRelationType::COUPLING, {}, rexsapi::TRelationReferences{}};

    CHECK_FALSE(checker.check(result, rexsapi::TRexsVersion{0, 9}, relation));
    CHECK_FALSE(result);
  }

  SUBCASE("Check relation not allowed")
  {
    rexsapi::TRelation relation{rexsapi::TRelationType::PLANET_SHAFT, {}, rexsapi::TRelationReferences{}};

    CHECK_FALSE(checker.check(result, rexsapi::TRexsVersion{1, 0}, relation));
    CHECK_FALSE(result);
    CHECK(result.hasIssues());
  }

  SUBCASE("Check relation ordering fail")
  {
    rexsapi::TRelationReferences references;
    references.emplace_back(rexsapi::TRelationReference{rexsapi::TRelationRole::PLANETARY_STAGE, "", comonent1});
    references.emplace_back(rexsapi::TRelationReference{rexsapi::TRelationRole::SHAFT, "", comonent2});
    rexsapi::TRelation relation{rexsapi::TRelationType::PLANET_SHAFT, 1, references};

    CHECK_FALSE(checker.check(result, rexsapi::TRexsVersion{1, 4}, relation));
    CHECK_FALSE(result);
  }

  SUBCASE("Check too many roles in relation")
  {
    rexsapi::TRelationReferences references;
    references.emplace_back(rexsapi::TRelationReference{rexsapi::TRelationRole::PLANETARY_STAGE, "", comonent1});
    references.emplace_back(rexsapi::TRelationReference{rexsapi::TRelationRole::SHAFT, "", comonent2});
    references.emplace_back(rexsapi::TRelationReference{rexsapi::TRelationRole::GEAR, "", comonent2});
    rexsapi::TRelation relation{rexsapi::TRelationType::PLANET_SHAFT, 1, references};

    CHECK_FALSE(checker.check(result, rexsapi::TRexsVersion{1, 4}, relation));
    CHECK_FALSE(result);
    CHECK(result.hasIssues());
  }

  SUBCASE("Load broken mapping")
  {
    rexsapi::detail::parseMappings(result, "no-json");
    CHECK_FALSE(result);
  }
}
