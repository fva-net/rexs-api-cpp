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

#ifndef TEST_TEST_MODEL_HXX
#define TEST_TEST_MODEL_HXX

#include <rexsapi/Model.hxx>
#include <rexsapi/database/Model.hxx>


static inline rexsapi::TModel createModel(const rexsapi::database::TModel& dbModel)
{
  uint64_t componentId = 1;
  rexsapi::TComponents components;
  const auto& gearUnitComponent = dbModel.findComponentById("gear_unit");
  rexsapi::TAttributes attributes;
  // BOOLEAN
  attributes.emplace_back(
    rexsapi::TAttribute{gearUnitComponent.findAttributeById("account_for_gravity"), rexsapi::TValue{true}});
  // INTEGER
  attributes.emplace_back(
    rexsapi::TAttribute{gearUnitComponent.findAttributeById("gear_shift_index"), rexsapi::TValue{5}});
  // DATE_TIME
  attributes.emplace_back(rexsapi::TAttribute{gearUnitComponent.findAttributeById("modification_date"),
                                              rexsapi::TValue{rexsapi::TDatetime{"2022-06-05T08:50:27+03:00"}}});

  components.emplace_back(rexsapi::TComponent{componentId++, gearUnitComponent, "Getriebe", std::move(attributes)});

  attributes = rexsapi::TAttributes{};
  const auto& couplingComponent = dbModel.findComponentById("coupling");
  // FLOATING_POINT
  attributes.emplace_back(
    rexsapi::TAttribute{couplingComponent.findAttributeById("mass_of_component"), rexsapi::TValue{3.52}});
  // FLOATING_POINT_ARRAY
  attributes.emplace_back(rexsapi::TAttribute{couplingComponent.findAttributeById("display_color"),
                                              rexsapi::TValue{rexsapi::TFloatArrayType{30.0, 10.0, 55.0}}});

  // FLOATING_POINT_ARRAY_CODED
  auto value = rexsapi::TValue{rexsapi::TFloatArrayType{2.0, 3.0, 4.0}};
  value.coded(rexsapi::TCodeType::Default);
  attributes.emplace_back(rexsapi::TAttribute{couplingComponent.findAttributeById("u_axis_vector"), value});

  // REFERENCE_COMPONENT
  attributes.emplace_back(rexsapi::TAttribute{couplingComponent.findAttributeById("reference_component_for_position"),
                                              rexsapi::TValue{rexsapi::TReferenceComponentType{1}}});
  components.emplace_back(rexsapi::TComponent{componentId++, couplingComponent, "Kupplung 1", std::move(attributes)});

  attributes = rexsapi::TAttributes{};
  const auto& switchableCouplingComponent = dbModel.findComponentById("switchable_coupling");
  // BOOLEAN_ARRAY
  attributes.emplace_back(rexsapi::TAttribute{switchableCouplingComponent.findAttributeById("is_engaged"),
                                              rexsapi::TValue{rexsapi::TBoolArrayType{true, false}}});
  components.emplace_back(
    rexsapi::TComponent{componentId++, switchableCouplingComponent, "Kupplung 2", std::move(attributes)});

  attributes = rexsapi::TAttributes{};
  const auto& conceptBearingComponent = dbModel.findComponentById("concept_bearing");
  // ENUM
  attributes.emplace_back(rexsapi::TAttribute{conceptBearingComponent.findAttributeById("axial_force_absorption"),
                                              rexsapi::TValue{"no_direction"}});
  components.emplace_back(rexsapi::TComponent{componentId++, conceptBearingComponent, "Lager", std::move(attributes)});

  attributes = rexsapi::TAttributes{};
  const auto& elementListComponent = dbModel.findComponentById("element_list");
  // ENUM_ARRAY
  attributes.emplace_back(rexsapi::TAttribute{elementListComponent.findAttributeById("element_types"),
                                              rexsapi::TValue{rexsapi::TEnumArrayType{"line3", "pyramid12"}}});
  // ARRAY_OF_INTEGER_ARRAYS
  attributes.emplace_back(rexsapi::TAttribute{elementListComponent.findAttributeById("element_structure"),
                                              rexsapi::TValue{rexsapi::TArrayOfIntArraysType{{1, 2, 3}, {4, 5}, {6}}}});
  // INTEGER_ARRAY
  attributes.emplace_back(rexsapi::TAttribute{elementListComponent.findAttributeById("element_ids"),
                                              rexsapi::TValue{rexsapi::TIntArrayType{{1, 2, 3}}}});
  // STRING_ARRAY
  attributes.emplace_back(rexsapi::TAttribute{"custom_string_array", rexsapi::TUnit{""},
                                              rexsapi::TValueType::STRING_ARRAY,
                                              rexsapi::TValue{rexsapi::TStringArrayType{"hutzli", "putzli"}}});
  // STRING_MATRIX
  attributes.emplace_back(
    rexsapi::TAttribute{"custom_string_matrix", rexsapi::TUnit{""}, rexsapi::TValueType::STRING_MATRIX,
                        rexsapi::TValue{rexsapi::TStringMatrixType{{{"hutzli", "putzli"}, {"putzli", "hutzli"}}}}});
  components.emplace_back(
    rexsapi::TComponent{componentId++, elementListComponent, "Element Typ", std::move(attributes)});

  attributes = rexsapi::TAttributes{};
  const auto& assemblyGroupComponent = dbModel.findComponentById("assembly_group");
  // FILE_REFERENCE
  attributes.emplace_back(rexsapi::TAttribute{assemblyGroupComponent.findAttributeById("folder"),
                                              rexsapi::TValue{rexsapi::TFileReferenceType("./out")}});
  // STRING
  attributes.emplace_back(
    rexsapi::TAttribute{assemblyGroupComponent.findAttributeById("fem_file_format"), rexsapi::TValue{"puschel"}});
  // FLOATING_POINT_MATRIX
  attributes.emplace_back(rexsapi::TAttribute{
    assemblyGroupComponent.findAttributeById("reduced_static_stiffness_matrix"),
    rexsapi::TValue{rexsapi::TFloatMatrixType{{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}}}}});
  // INTEGER_MATRIX
  attributes.emplace_back(
    rexsapi::TAttribute{"custom_integer_matrix", rexsapi::TUnit{""}, rexsapi::TValueType::INTEGER_MATRIX,
                        rexsapi::TValue{rexsapi::TIntMatrixType{{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}}}});
  components.emplace_back(
    rexsapi::TComponent{componentId++, assemblyGroupComponent, "Assembly", std::move(attributes)});

  attributes = rexsapi::TAttributes{};
  const auto& assemblyGroupComponent2 = dbModel.findComponentById("assembly_group");
  // FLOATING_POINT_MATRIX_CODED
  value = rexsapi::TValue{rexsapi::TFloatMatrixType{{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}}}};
  value.coded(rexsapi::TCodeType::Optimized);
  attributes.emplace_back(
    rexsapi::TAttribute{assemblyGroupComponent2.findAttributeById("reduced_static_stiffness_matrix"), value});
  // BOOLEAN_MATRIX
  attributes.emplace_back(
    rexsapi::TAttribute{"custom_boolean_matrix", rexsapi::TUnit{""}, rexsapi::TValueType::BOOLEAN_MATRIX,
                        rexsapi::TValue{rexsapi::TBoolMatrixType{{{true, false}, {false, true}}}}});
  components.emplace_back(
    rexsapi::TComponent{componentId++, assemblyGroupComponent2, "Assembly", std::move(attributes)});

  // Relations
  rexsapi::TRelations relations;
  relations.emplace_back(rexsapi::TRelation{
    rexsapi::TRelationType::ASSEMBLY,
    {},
    rexsapi::TRelationReferences{rexsapi::TRelationReference{rexsapi::TRelationRole::ASSEMBLY, "hint0", components[0]},
                                 rexsapi::TRelationReference{rexsapi::TRelationRole::PART, "hint1", components[1]}}});
  relations.emplace_back(
    rexsapi::TRelation{rexsapi::TRelationType::REFERENCE,
                       {},
                       rexsapi::TRelationReferences{
                         rexsapi::TRelationReference{rexsapi::TRelationRole::REFERENCED, "hint2", components[2]},
                         rexsapi::TRelationReference{rexsapi::TRelationRole::ORIGIN, "hint3", components[3]}}});
  relations.emplace_back(rexsapi::TRelation{
    rexsapi::TRelationType::MANUFACTURING_STEP, 1,
    rexsapi::TRelationReferences{
      rexsapi::TRelationReference{rexsapi::TRelationRole::TOOL, "hint4", components[4]},
      rexsapi::TRelationReference{rexsapi::TRelationRole::WORKPIECE, "hint5", components[5]},
      rexsapi::TRelationReference{rexsapi::TRelationRole::MANUFACTURING_SETTINGS, "hint6", components[6]}}});

  // Load Spectrum
  rexsapi::TAttributes loadAttributes;
  loadAttributes.emplace_back(
    rexsapi::TAttribute{gearUnitComponent.findAttributeById("gravitational_acceleration"), rexsapi::TValue{10.5}});
  loadAttributes.emplace_back(
    rexsapi::TAttribute{gearUnitComponent.findAttributeById("reference_temperature"), rexsapi::TValue{55.5}});

  rexsapi::TLoadComponents loadComponents;
  loadComponents.emplace_back(rexsapi::TLoadComponent{components[0], std::move(loadAttributes)});

  loadAttributes = rexsapi::TAttributes{};
  loadAttributes.emplace_back(
    rexsapi::TAttribute{couplingComponent.findAttributeById("mass_of_component"), rexsapi::TValue{5.5}});
  loadComponents.emplace_back(rexsapi::TLoadComponent{components[1], std::move(loadAttributes)});
  rexsapi::TLoadCase loadCase{std::move(loadComponents)};
  rexsapi::TLoadCases loadCases{std::move(loadCase)};

  rexsapi::TLoadComponents accumulationComponents;
  loadAttributes = rexsapi::TAttributes{};
  loadAttributes.emplace_back(
    rexsapi::TAttribute{gearUnitComponent.findAttributeById("operating_time"), rexsapi::TValue{15.5}});
  loadAttributes.emplace_back(
    rexsapi::TAttribute{gearUnitComponent.findAttributeById("gravitational_acceleration"), rexsapi::TValue{0.99}});
  accumulationComponents.emplace_back(rexsapi::TLoadComponent{components[0], std::move(loadAttributes)});

  rexsapi::TAccumulation accumulation{std::move(accumulationComponents)};

  rexsapi::TLoadSpectrum loadSpectrum{std::move(loadCases), std::move(accumulation)};

  rexsapi::TModelInfo info{"REXSApi Unit Test", "1.0", "2022-05-20T08:59:10+01:00", dbModel.getVersion(), "en"};

  return rexsapi::TModel{info, std::move(components), std::move(relations), std::move(loadSpectrum)};
}

#endif
