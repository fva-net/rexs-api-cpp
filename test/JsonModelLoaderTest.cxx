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
    static const rexsapi::TFileJsonSchemaLoader schemaLoader{projectDir() / "models" / "rexs-schema.json"};
    static const rexsapi::TJsonSchemaValidator jsonValidator{schemaLoader};

    rexsapi::detail::TFileModelLoader<rexsapi::TJsonSchemaValidator, rexsapi::TJsonModelLoader> loader{jsonValidator,
                                                                                                       path};
    return loader.load(mode, result, registry);
  }

  const std::string MemModel = R"({
  "model":{
    "applicationId":"Bearinx",
    "applicationVersion":"12.0.8823",
    "date":"2021-07-01T12:18:38+01:00",
    "version":"1.5",
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
            "id":"modification_date",
            "date_time":"2023-03-29T11:46:00+02:00"
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

std::optional<rexsapi::TModel> loadModelBuffer(rexsapi::TResult& result, const std::string& buffer,
                                               const rexsapi::database::TModelRegistry& registry,
                                               rexsapi::TMode mode = rexsapi::TMode::STRICT_MODE)
{
  static const rexsapi::TFileJsonSchemaLoader schemaLoader{projectDir() / "models" / "rexs-schema.json"};
  static const rexsapi::TJsonSchemaValidator validator{schemaLoader};

  rexsapi::detail::TBufferModelLoader<rexsapi::TJsonSchemaValidator, rexsapi::TJsonModelLoader> loader{validator,
                                                                                                       buffer};

  return loader.load(mode, result, registry);
}


TEST_CASE("Json model loader test")
{
  rexsapi::TResult result;
  const auto registry = createModelRegistry();

  SUBCASE("Load valid document from buffer")
  {
    const auto model = loadModelBuffer(result, MemModel, registry, rexsapi::TMode::RELAXED_MODE);
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 4);
    CHECK(result.getErrors()[0].getMessage() ==
          "Transmission unit: attribute id=matrix_correction is not part of component gear_unit id=1");
    CHECK(result.getErrors()[1].getMessage() ==
          "Transmission unit: attribute id=element_structure is not part of component gear_unit id=1");
    CHECK(result.getErrors()[2].getMessage() ==
          "Welle 1: duplicate attribute found for attribute id=display_color of component id=2");
    CHECK(result.getErrors()[3].getMessage() ==
          "load_case id=1: attribute id=load_duration_fraction is not part of component gear_unit id=1");
    CHECK_FALSE(result.isCritical());
    REQUIRE(model);
    CHECK(model->getInfo().getVersion() == rexsapi::TRexsVersion{1, 5});
    REQUIRE(model->getComponents().size() == 4);
    CHECK(model->getComponents()[0].getName() == "Transmission unit");
    REQUIRE(model->getComponents()[0].getAttributes().size() == 8);
    CHECK(model->getComponents()[0].getAttributes()[4].getName() == "Modification date");
    CHECK(model->getComponents()[0].getAttributes()[4].getValue<rexsapi::TDatetime>().asUTCString() ==
          "2023-03-29T09:46:00+00:00");
    REQUIRE(model->getRelations().size() == 2);
    CHECK(model->getLoadSpectrum().hasLoadCases());
    REQUIRE(model->getLoadSpectrum().getLoadCases().size() == 1);
    REQUIRE(model->getLoadSpectrum().getLoadCases()[0].getLoadComponents().size() == 2);
    CHECK(model->getLoadSpectrum().getLoadCases()[0].getLoadComponents()[0].getAttributes().size() == 9);
    CHECK(model->getLoadSpectrum().getLoadCases()[0].getLoadComponents()[0].getLoadAttributes().size() == 1);
    REQUIRE(model->getLoadSpectrum().hasAccumulation());
  }

  SUBCASE("Load complex model from file in strict mode")
  {
    const auto model =
      loadModel(result, projectDir() / "test" / "example_models" / "FVA-Industriegetriebe_2stufig_1-4.rexsj", registry,
                rexsapi::TMode::STRICT_MODE);
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
    const auto model =
      loadModel(result, projectDir() / "test" / "example_models" / "FVA-Industriegetriebe_2stufig_1-4.rexsj", registry,
                rexsapi::TMode::RELAXED_MODE);
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
    const auto model = loadModel(result, projectDir() / "test" / "example_models" / "FVA_worm_stage_1-4.rexsj",
                                 registry, rexsapi::TMode::RELAXED_MODE);
    REQUIRE(model);
    CHECK(result);
    CHECK_FALSE(result.isCritical());
    REQUIRE(result.getErrors().size() == 5);
  }

  SUBCASE("Load model from non-existent file failure")
  {
    const auto model = loadModel(result, "non-exising-file.rexsj", registry);
    CHECK_FALSE(model);
    CHECK_FALSE(result);
    CHECK(result.isCritical());
    REQUIRE(result.getErrors().size() == 1);
    CHECK(result.getErrors()[0].getMessage() == "'non-exising-file.rexsj' does not exist");
  }

  SUBCASE("Load model from directory failure")
  {
    const auto model = loadModel(result, projectDir(), registry);
    CHECK_FALSE(model);
    CHECK_FALSE(result);
    CHECK(result.isCritical());
    REQUIRE(result.getErrors().size() == 1);
  }


  SUBCASE("Load incorrect json document")
  {
    const std::string buffer = R"({
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

    const auto model = loadModelBuffer(result, buffer, registry, rexsapi::TMode::RELAXED_MODE);
    CHECK(result);
    CHECK(result.getErrors().size() == 10);
    CHECK_FALSE(result.isCritical());
    CHECK(model);
  }

  SUBCASE("Load invalid json document")
  {
    const std::string buffer = R"({
  "model":{
    "applicationId":"Bearinx",
    "applicationVersion":"12.0.8823",
    "date":"2021-07-01T12:18:38+01:00",
    "version":"1.4",
    "relations":[
    ]}
  })";

    const auto model = loadModelBuffer(result, buffer, registry, rexsapi::TMode::RELAXED_MODE);
    CHECK_FALSE(result);
    CHECK(result.isCritical());
    CHECK_FALSE(model);
  }

  SUBCASE("Load broken json document")
  {
    const std::string buffer = R"({
  "model":{
    "applicationId":"Bearinx",
    "applicationVersion":"12.0.8823",
    "date":"2021-07-01T12:18:38+01:00",
  })";

    const auto model = loadModelBuffer(result, buffer, registry, rexsapi::TMode::RELAXED_MODE);
    CHECK_FALSE(result);
    CHECK(result.isCritical());
    CHECK_FALSE(model);
  }

  SUBCASE("Load valid document with unkown version")
  {
    std::string buffer{MemModel};
    replace(buffer, R"("version":"1.5")", R"("version":"1.99")");
    const auto model = loadModelBuffer(result, buffer, registry, rexsapi::TMode::RELAXED_MODE);
    CHECK_FALSE(result);
    CHECK_FALSE(result.isCritical());
    REQUIRE(model);
    result.reset();

    CHECK_THROWS((void)loadModelBuffer(result, buffer, registry, rexsapi::TMode::STRICT_MODE));
  }
}
