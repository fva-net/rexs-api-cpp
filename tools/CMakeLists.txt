
add_executable(model_checker
  ModelChecker.cxx
  ToolsHelper.hxx
)

if(MSVC)
  target_compile_options(model_checker PRIVATE /bigobj)
endif()

target_link_libraries(model_checker PRIVATE
  rexsapi CLI11::CLI11
)

add_executable(model_converter
  ModelConverter.cxx
  ToolsHelper.hxx
)

if(MSVC)
  target_compile_options(model_converter PRIVATE /bigobj)
endif()

target_link_libraries(model_converter PRIVATE
  rexsapi CLI11::CLI11
)

add_executable(model_dumper
  ModelDumper.cxx
  ToolsHelper.hxx
)

if(MSVC)
  target_compile_options(model_dumper PRIVATE /bigobj)
endif()

target_link_libraries(model_dumper PRIVATE
  rexsapi CLI11::CLI11
)

install(TARGETS model_checker RUNTIME DESTINATION tools)
install(TARGETS model_converter RUNTIME DESTINATION tools)
install(TARGETS model_dumper RUNTIME DESTINATION tools)
