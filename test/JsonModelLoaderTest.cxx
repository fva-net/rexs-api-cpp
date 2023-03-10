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

#include <rexsapi/ModelLoader.hxx>

#include <test/TestHelper.hxx>
#include <test/TestModelHelper.hxx>
#include <test/TestModelLoader.hxx>

#include <doctest.h>

namespace
{
  std::optional<rexsapi::TModel> loadModel(rexsapi::TResult& result, const std::filesystem::path& path,
                                           const rexsapi::database::TModelRegistry& registry,
                                           rexsapi::TMode mode = rexsapi::TMode::STRICT_MODE)
  {
    rexsapi::TFileJsonSchemaLoader schemaLoader{projectDir() / "models" / "rexs-schema.json"};
    rexsapi::TJsonSchemaValidator jsonValidator{schemaLoader};

    rexsapi::detail::TFileModelLoader<rexsapi::TJsonSchemaValidator, rexsapi::TJsonModelLoader> loader{jsonValidator,
                                                                                                       path};
    return loader.load(mode, result, registry);
  }

  const std::string MemModel = R"({
  "model":{
    "applicationId":"Bearinx",
    "applicationVersion":"12.0.8823",
    "date":"2021-07-01T12:18:38+01:00",
    "version":"1.4",
    "relations":[
      {
        "id":48,
        "type":"assembly",
        "refs":[
          {
            "hint":"gear_unit",
            "id":1,
            "role":"assembly"
          },
          {
            "hint":"gear_casing",
            "id":4,
            "role":"part"
          }
        ]
      },
      {
        "id":49,
        "type":"side",
        "refs":[
          {
            "hint":"concept_bearing",
            "id":3,
            "role":"assembly"
          },
          {
            "hint":"shaft",
            "id":2,
            "role":"inner_part"
          },
          {
            "hint":"gear_casing",
            "id":4,
            "role":"outer_part"
          }
        ]
      }
    ],
    "components":[
      {
        "id":1,
        "name":"Transmission unit",
        "type":"gear_unit",
        "attributes":[
          {
            "id":"reference_temperature",
            "unit":"C",
            "floating_point":17.0
          },
          {
            "id":"gear_shift_index",
            "unit":"none",
            "integer":7
          },
          {
            "id":"number_of_gears",
            "integer":3
          },
          {
            "id":"account_for_gravity",
            "boolean":true
          },
          {
            "id":"u_axis_vector",
            "unit":"mm",
            "floating_point_array":[1.0, 0.0, 0.0]
          },
          {
            "id":"matrix_correction",
            "unit":"mm",
            "floating_point_matrix":[
              [1.0, 0.0, 0.0],
              [0.0, 1.0, 0.0],
              [0.0, 0.0, 1.0]
            ]
          },
          {
            "id":"element_structure",
            "array_of_integer_arrays":[
              [108, 2, 1, 107, 7],
              [0, 1, 0],
              [0, 7, 0, 1]
            ]
          }
        ]
      },
      {
        "id":2,
        "name":"Welle 1",
        "type":"shaft",
        "attributes":[
          {
            "id":"display_color",
            "unit":"%",
            "floating_point_array":[0.90, 0.80, 0.70]
          },
          {
            "id":"u_axis_vector",
            "unit":"mm",
            "floating_point_array_coded":{"code": "float32", "value": "AADgQAAAAEEAABBB"}
          },
          {
            "id":"display_color",
            "unit":"%",
            "floating_point_array":[0.90, 0.80, 0.70]
          }
        ]
      },
      {
        "id":3,
        "name": "Lager",
        "type": "concept_bearing",
        "attributes":[]
      },
      {
        "id":4,
        "name": "Gehäuse",
        "type": "gear_casing",
        "attributes":[]
      }
    ],
    "load_spectrum": {
      "id": 1,
      "load_cases": [
        {
          "id": 1,
          "components": [
            {
              "id": 1,
              "type": "gear_unit",
              "name":"Transmission unit",
              "attributes": [
                { "id": "load_duration_fraction", "unit": "%", "floating_point": 15 }
              ]
            },
            {
              "id": 2,
              "attributes": [
                { "id": "custom_load_duration_fraction", "unit": "%", "floating_point": 21 }
              ]
            }
          ]
        }
      ],
      "accumulation": {
        "components": [
            {
              "id": 1,
              "type": "gear_unit",
              "attributes": [
                { "id": "operating_time", "floating_point": 35.8 }
              ]
            }          
        ]
      }
    }
  }
})";
}

TEST_CASE("Json model loader test")
{
  rexsapi::TFileJsonSchemaLoader schemaLoader{projectDir() / "models" / "rexs-schema.json"};
  rexsapi::TJsonSchemaValidator validator{schemaLoader};
  rexsapi::TResult result;
  const auto registry = createModelRegistry();

  SUBCASE("Load valid document from buffer")
  {
    rexsapi::detail::TBufferModelLoader<rexsapi::TJsonSchemaValidator, rexsapi::TJsonModelLoader> loader{validator,
                                                                                                         MemModel};
    auto model = loader.load(rexsapi::TMode::RELAXED_MODE, result, registry);
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 4);
    CHECK(result.getErrors()[0].getMessage() ==
          "Transmission unit: attribute id=matrix_correction is not part of component gear_unit id=1");
    CHECK(result.getErrors()[1].getMessage() ==
          "Transmission unit: attribute id=element_structure is not part of component gear_unit id=1");
    CHECK(result.getErrors()[2].getMessage() == "Welle 1: duplicate attribute found for attribute id=display_color");
    CHECK(result.getErrors()[3].getMessage() ==
          "load_case id=1: attribute id=load_duration_fraction is not part of component gear_unit id=1");
    CHECK_FALSE(result.isCritical());
    REQUIRE(model);
    REQUIRE(model->getComponents().size() == 4);
    REQUIRE(model->getRelations().size() == 2);
    CHECK(model->getLoadSpectrum().hasLoadCases());
    REQUIRE(model->getLoadSpectrum().getLoadCases().size() == 1);
    REQUIRE(model->getLoadSpectrum().getLoadCases()[0].getLoadComponents().size() == 2);
    CHECK(model->getLoadSpectrum().getLoadCases()[0].getLoadComponents()[0].getAttributes().size() == 8);
    CHECK(model->getLoadSpectrum().getLoadCases()[0].getLoadComponents()[0].getLoadAttributes().size() == 1);
    REQUIRE(model->getLoadSpectrum().hasAccumulation());
  }

  SUBCASE("Load complex model from file in strict mode")
  {
    auto model = loadModel(result, projectDir() / "test" / "example_models" / "FVA-Industriegetriebe_2stufig_1-4.rexsj",
                           registry, rexsapi::TMode::STRICT_MODE);
    REQUIRE(model);
    CHECK_FALSE(result);
    CHECK_FALSE(result.isCritical());
    REQUIRE(result.getErrors().size() == 10);
    CHECK(result.getErrors()[0].getMessage() ==
          "Gear unit [1]: attribute id=EIGENGEWICHT is not part of component gear_unit id=1");
    CHECK(result.getErrors()[1].getMessage() == "6210-2Z (Rolling bearing [33]): value is out of range for attribute "
                                                "id=u_coordinate_on_shaft_outer_side of component id=33");
    CHECK(result.getErrors()[2].getMessage() ==
          "not a catalogue bearing: 33016 (Rolling bearing [35]): value is out of range for attribute "
          "id=u_coordinate_on_shaft_outer_side of component id=37");
    CHECK(result.getErrors()[3].getMessage() ==
          "Material 1: value is out of range for attribute id=thermal_expansion_coefficient_minus of component id=57");
    CHECK(result.getErrors()[4].getMessage() ==
          "Material 2: value is out of range for attribute id=thermal_expansion_coefficient_minus of component id=58");
    CHECK(result.getErrors()[5].getMessage() ==
          "Material 3: value is out of range for attribute id=thermal_expansion_coefficient_minus of component id=59");
    CHECK(result.getErrors()[6].getMessage() ==
          "load_case id=1: attribute id=load_duration_fraction is not part of component gear_unit id=1");
    CHECK(result.getErrors()[7].getMessage() ==
          "load_case id=2: attribute id=load_duration_fraction is not part of component gear_unit id=1");
    CHECK(result.getErrors()[8].getMessage() ==
          "load_case id=3: attribute id=load_duration_fraction is not part of component gear_unit id=1");
    CHECK(result.getErrors()[9].getMessage() ==
          "load_case id=4: attribute id=load_duration_fraction is not part of component gear_unit id=1");

    const auto& attributes =
      AttributeFinder(ComponentFinder(*model).findComponent("Gear unit [1]")).findCustomAttributes();
    CHECK(attributes.size() == 2);
  }

  SUBCASE("Load complex model from file in relaxed mode")
  {
    auto model = loadModel(result, projectDir() / "test" / "example_models" / "FVA-Industriegetriebe_2stufig_1-4.rexsj",
                           registry, rexsapi::TMode::RELAXED_MODE);
    REQUIRE(model);
    CHECK(result);
    CHECK_FALSE(result.isCritical());
    REQUIRE(result.getErrors().size() == 10);
    CHECK(result.getErrors()[0].getMessage() ==
          "Gear unit [1]: attribute id=EIGENGEWICHT is not part of component gear_unit id=1");
    CHECK(result.getErrors()[1].getMessage() == "6210-2Z (Rolling bearing [33]): value is out of range for attribute "
                                                "id=u_coordinate_on_shaft_outer_side of component id=33");
    CHECK(result.getErrors()[2].getMessage() ==
          "not a catalogue bearing: 33016 (Rolling bearing [35]): value is out of range for attribute "
          "id=u_coordinate_on_shaft_outer_side of component id=37");
    CHECK(result.getErrors()[3].getMessage() ==
          "Material 1: value is out of range for attribute id=thermal_expansion_coefficient_minus of component id=57");
    CHECK(result.getErrors()[4].getMessage() ==
          "Material 2: value is out of range for attribute id=thermal_expansion_coefficient_minus of component id=58");
    CHECK(result.getErrors()[5].getMessage() ==
          "Material 3: value is out of range for attribute id=thermal_expansion_coefficient_minus of component id=59");
    CHECK(result.getErrors()[6].getMessage() ==
          "load_case id=1: attribute id=load_duration_fraction is not part of component gear_unit id=1");
    CHECK(result.getErrors()[7].getMessage() ==
          "load_case id=2: attribute id=load_duration_fraction is not part of component gear_unit id=1");
    CHECK(result.getErrors()[8].getMessage() ==
          "load_case id=3: attribute id=load_duration_fraction is not part of component gear_unit id=1");
    CHECK(result.getErrors()[9].getMessage() ==
          "load_case id=4: attribute id=load_duration_fraction is not part of component gear_unit id=1");

    const auto& attributes =
      AttributeFinder(ComponentFinder(*model).findComponent("Gear unit [1]")).findCustomAttributes();
    CHECK(attributes.size() == 2);
  }

  SUBCASE("Load complex model without loadmodel from file")
  {
    auto model = loadModel(result, projectDir() / "test" / "example_models" / "FVA_worm_stage_1-4.rexsj", registry,
                           rexsapi::TMode::RELAXED_MODE);
    REQUIRE(model);
    CHECK(result);
    CHECK_FALSE(result.isCritical());
    REQUIRE(result.getErrors().size() == 5);
  }

  SUBCASE("Load model from non-existent file failure")
  {
    auto model = loadModel(result, "non-exising-file.rexsj", registry);
    CHECK_FALSE(model);
    CHECK_FALSE(result);
    CHECK(result.isCritical());
    REQUIRE(result.getErrors().size() == 1);
    CHECK(result.getErrors()[0].getMessage() == "'non-exising-file.rexsj' does not exist");
  }

  SUBCASE("Load model from directory failure")
  {
    auto model = loadModel(result, projectDir(), registry);
    CHECK_FALSE(model);
    CHECK_FALSE(result);
    CHECK(result.isCritical());
    REQUIRE(result.getErrors().size() == 1);
  }


  SUBCASE("Load incorrect json document")
  {
    std::string buffer = R"({
  "model":{
    "applicationId":"Bearinx",
    "applicationVersion":"12.0.8823",
    "date":"2021-07-01T12:18:38+01:00",
    "version":"1.4",
    "relations":[
      {
        "id":48,
        "type":"assembly",
        "refs":[
          {
            "hint":"gear_unit",
            "id":1,
            "role":"assembly"
          },
          {
            "hint":"gear_casing",
            "id":2,
            "role":"part"
          }
        ]
      },
      {
        "id":49,
        "type":"hutzli",
        "refs":[
          {
            "hint":"gear_unit",
            "id":1,
            "role":"assembly"
          }
        ]
      },
      {
        "id":50,
        "type":"assembly",
        "refs":[
          {
            "hint":"gear_unit",
            "id":1,
            "role":"puschel"
          }
        ]
      }
    ],
    "components": [
      {
        "id":1,
        "name":"Transmission unit",
        "type":"gear_unit",
        "attributes":[
          {
            "id":"reference_temperature",
            "unit":"kg",
            "floating_point":17.0
          },
          {
            "id":"gear_shift_index",
            "unit":"none",
            "boolean":true
          }
        ]
      },
      {
        "id":2,
        "name":"Welle 1",
        "type":"shaft",
        "attributes":[
          {
            "id":"display_color",
            "unit":"%",
            "floating_point_array":[0.90, 0.80, 0.70]
          },
          {
            "id":"u_axis_vector",
            "unit":"mm",
            "floating_point_array":[1.0,0.0,0.0]
          }
        ]
      },
      {
        "id":3,
        "name":"Welle 2",
        "type":"shaft",
        "attributes":[]
      }
    ],
    "load_spectrum": {
      "id": 1,
      "load_cases": [
        {
          "id": 1,
          "components": [
            {
              "id": 10,
              "type": "gear_unit",
              "name":"Transmission unit",
              "attributes": [
                { "id": "load_duration_fraction", "unit": "%", "floating_point": 15 }
              ]
            }
          ]
        }
      ],
      "accumulation": {
        "components": [
            {
              "id": 11,
              "type": "gear_unit",
              "attributes": [
                { "id": "operating_time", "floating_point": 35.8 }
              ]
            }
        ]
      }
    }
  }
})";

    rexsapi::detail::TBufferModelLoader<rexsapi::TJsonSchemaValidator, rexsapi::TJsonModelLoader> loader{validator,
                                                                                                         buffer};
    auto model = loader.load(rexsapi::TMode::RELAXED_MODE, result, registry);
    CHECK(result);
    CHECK(result.getErrors().size() == 10);
    CHECK_FALSE(result.isCritical());
    CHECK(model);
  }

  SUBCASE("Load invalid json document")
  {
    std::string buffer = R"({
  "model":{
    "applicationId":"Bearinx",
    "applicationVersion":"12.0.8823",
    "date":"2021-07-01T12:18:38+01:00",
    "version":"1.4",
    "relations":[
    ]}
  })";

    rexsapi::detail::TBufferModelLoader<rexsapi::TJsonSchemaValidator, rexsapi::TJsonModelLoader> loader{validator,
                                                                                                         buffer};
    auto model = loader.load(rexsapi::TMode::RELAXED_MODE, result, registry);
    CHECK_FALSE(result);
    CHECK(result.isCritical());
    CHECK_FALSE(model);
  }

  SUBCASE("Load broken json document")
  {
    std::string buffer = R"({
  "model":{
    "applicationId":"Bearinx",
    "applicationVersion":"12.0.8823",
    "date":"2021-07-01T12:18:38+01:00",
  })";

    rexsapi::detail::TBufferModelLoader<rexsapi::TJsonSchemaValidator, rexsapi::TJsonModelLoader> loader{validator,
                                                                                                         buffer};
    auto model = loader.load(rexsapi::TMode::RELAXED_MODE, result, registry);
    CHECK_FALSE(result);
    CHECK(result.isCritical());
    CHECK_FALSE(model);
  }

  SUBCASE("Load valid document with unkown version")
  {
    std::string buffer{MemModel};
    replace(buffer, R"("version":"1.4")", R"("version":"1.99")");
    rexsapi::detail::TBufferModelLoader<rexsapi::TJsonSchemaValidator, rexsapi::TJsonModelLoader> loader{validator,
                                                                                                         buffer};
    auto model = loader.load(rexsapi::TMode::RELAXED_MODE, result, registry);
    CHECK_FALSE(result);
    CHECK_FALSE(result.isCritical());
    REQUIRE(model);
    result.reset();

    CHECK_THROWS((void)loader.load(rexsapi::TMode::STRICT_MODE, result, registry));
  }
}
