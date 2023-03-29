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

#include <rexsapi/ModelVisitor.hxx>

#include <test/TestModel.hxx>
#include <test/TestModelLoader.hxx>

#include <doctest.h>

namespace
{
  class MyModelVisitor : public rexsapi::TModelVisitor
  {
  public:
    MyModelVisitor() = default;

  private:
    void onVisit(const rexsapi::TModelInfo&)
    {
      ++noInfo;
    }

    void onVisit(const rexsapi::TRelation&)
    {
      ++noRelations;
    }

    void onVisit(const rexsapi::TRelationReference&)
    {
      ++noReferences;
    }

    void onVisit(const rexsapi::TComponent&)
    {
      ++noComponents;
    }

    void onVisit(const rexsapi::TAttribute&)
    {
      ++noAttributes;
    }

    void onVisit(const rexsapi::TLoadSpectrum&)
    {
      ++noSpectrum;
    }

    void onVisit(const rexsapi::TLoadCase&)
    {
      ++noCases;
    }

    void onVisit(const rexsapi::TAccumulation&)
    {
      ++noAccumulation;
    }

    void onVisit(const rexsapi::TLoadComponent&)
    {
      ++noLoadComponents;
    }

  public:
    size_t noInfo{0};
    size_t noRelations{0};
    size_t noReferences{0};
    size_t noComponents{0};
    size_t noAttributes{0};
    size_t noSpectrum{0};
    size_t noCases{0};
    size_t noAccumulation{0};
    size_t noLoadComponents{0};
  };
}


TEST_CASE("Model visitor")
{
  const auto model = createModel(loadModel("1.5"));

  SUBCASE("Visit model")
  {
    MyModelVisitor visitor;
    visitor.visit(model);

    CHECK(visitor.noInfo == 1);
    CHECK(visitor.noRelations == 3);
    CHECK(visitor.noReferences == 7);
    CHECK(visitor.noComponents == 7);
    CHECK(visitor.noAttributes == 20);
    CHECK(visitor.noSpectrum == 1);
    CHECK(visitor.noCases == 1);
    CHECK(visitor.noAccumulation == 1);
    CHECK(visitor.noLoadComponents == 3);
  }
}
