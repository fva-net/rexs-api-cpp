
#define REXSAPI_MINIZ_IMPL
#include <rexsapi/Rexsapi.hxx>

#include <test/TestHelper.hxx>

#include <iostream>


static rexsapi::TModel createModel(const rexsapi::database::TModelRegistry& registry)
{
  const rexsapi::database::TModel& databaseModel = registry.getModel(rexsapi::TRexsVersion{"1.4"}, "en");

  rexsapi::TComponentBuilder componentBuilder{databaseModel};

  auto casingId = componentBuilder.addComponent("gear_casing").name("Gehäuse").id();
  componentBuilder.addAttribute("temperature_lubricant").unit("C").value(73.2);
  componentBuilder.addAttribute("type_of_gear_casing_construction_vdi_2736_2014").value("closed");
  componentBuilder.addCustomAttribute("custom_load_duration_fraction", rexsapi::TValueType::FLOATING_POINT)
    .unit("%")
    .value(30.0);

  auto lubricantId = componentBuilder.addComponent("lubricant").name("S2/220").id();
  componentBuilder.addAttribute("density_at_15_degree_celsius").unit("kg / dm^3").value(1.02);
  componentBuilder.addAttribute("lubricant_type_iso_6336_2006").value("non_water_soluble_polyglycol");
  componentBuilder.addAttribute("name").value("PG");
  componentBuilder.addAttribute("viscosity_at_100_degree_celsius").unit("mm^2 / s").value(37.0);
  componentBuilder.addAttribute("viscosity_at_40_degree_celsius").unit("mm^2 / s").value(220.0);

  auto flankDataId = componentBuilder.addComponent("gear_flank_data_set").id();
  componentBuilder.addAttribute("topographical_deviation_normals")
    .value(rexsapi::TFloatMatrixType{{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}}})
    .coded();

  rexsapi::TModelBuilder modelBuilder{std::move(componentBuilder)};

  modelBuilder.addComponent("concept_bearing", "my-bearing-id").name("Wälzlager");
  modelBuilder.addAttribute("axial_force_absorption").value("negative");
  modelBuilder.addAttribute("inner_diameter").unit("mm").value(30.0);
  modelBuilder.addAttribute("outer_diameter").unit("mm").value(62.0);
  modelBuilder.addAttribute("radial_force_absorption").value(true);
  modelBuilder.addAttribute("width").unit("mm").value(16.0);
  modelBuilder.addAttribute("misalignment_in_v_direction").unit("mum").value(0.0);
  modelBuilder.addAttribute("misalignment_in_w_direction").unit("mum").value(0.0);
  modelBuilder.addAttribute("support_vector").unit("mm").value(rexsapi::TFloatArrayType{70.0, 0.0, 0.0}).coded();
  modelBuilder.addAttribute("u_axis_vector").unit("mm").value(rexsapi::TFloatArrayType{1.0, 0.0, 0.0});
  modelBuilder.addAttribute("u_coordinate_on_shaft_inner_side").unit("mm").value(70.0);
  modelBuilder.addAttribute("u_coordinate_on_shaft_outer_side").unit("mm").value(70.0);
  modelBuilder.addAttribute("w_axis_vector")
    .unit("mm")
    .value(rexsapi::TFloatArrayType{0.0, 0.0, 1.0})
    .coded(rexsapi::TCodeType::Optimized);
  modelBuilder.addAttribute("axial_stiffness").unit("N / m").value(1.0E20);
  modelBuilder.addAttribute("radial_stiffness").unit("N / m").value(1.0E20);
  modelBuilder.addAttribute("bending_stiffness").unit("N mm / rad").value(0.0);
  modelBuilder.addAttribute("reference_component_for_position").reference("my-bearing-id");

  modelBuilder.addRelation(rexsapi::TRelationType::REFERENCE)
    .addRef(rexsapi::TRelationRole::ORIGIN, casingId)
    .addRef(rexsapi::TRelationRole::REFERENCED, lubricantId);
  modelBuilder.addRelation(rexsapi::TRelationType::FLANK)
    .addRef(rexsapi::TRelationRole::GEAR, "my-bearing-id")
    .addRef(rexsapi::TRelationRole::LEFT, lubricantId)
    .addRef(rexsapi::TRelationRole::RIGHT, flankDataId);

  auto& loadCase = modelBuilder.addLoadCase();
  loadCase.addComponent(casingId).addAttribute("temperature_lubricant").unit("C").value(36.7);
  loadCase.addAttribute("operating_viscosity").value(3.3);
  loadCase.addComponent("my-bearing-id").addAttribute("mass_of_component").value(2.35);

  auto& accumulation = modelBuilder.addAccumulation();
  accumulation.addComponent(casingId).addAttribute("operating_viscosity").value(1.25);

  return modelBuilder.build("REXSApi Model Builder", "1.0", "en");
}


int main(int argc, char** argv)
{
  try {
    auto registry = rexsapi::createModelRegistry(projectDir() / "models");
    auto model = createModel(registry);
    rexsapi::XMLModelSerializer modelSerializer;

    if (argc > 1) {
      rexsapi::TXMLFileSerializer serializer{argv[1]};
      modelSerializer.serialize(model, serializer);
      std::cout << "Model stored to " << argv[1];
    } else {
      rexsapi::TXMLStringSerializer serializer;
      modelSerializer.serialize(model, serializer);
      std::cout << serializer.getModel() << std::endl;
    }
  } catch (const std::exception& ex) {
    std::cerr << "Exception: " << ex.what() << std::endl;
  }
  return 0;
}
