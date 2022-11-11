
add_executable(builder_example
  BuilderExample.cxx
)

if(MSVC)
  target_compile_options(builder_example PRIVATE /W4 /WX /bigobj)
endif()

target_link_libraries(builder_example PRIVATE
  rexsapi
)
