FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/archive/refs/tags/v3.10.5.tar.gz
)

FetchContent_GetProperties(json)

if(NOT json_POPULATED)
  FetchContent_Populate(json)
endif()
