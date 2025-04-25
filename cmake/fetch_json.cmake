FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/archive/refs/tags/v3.12.0.tar.gz
)

FetchContent_GetProperties(json)

if(NOT json_POPULATED)
  FetchContent_Populate(json)
endif()
