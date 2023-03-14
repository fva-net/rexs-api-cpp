FetchContent_Declare(
  valijson
  URL https://github.com/tristanpenman/valijson/archive/refs/tags/v1.0.tar.gz
)

FetchContent_GetProperties(valijson)

if(NOT valijson_POPULATED)
  FetchContent_Populate(valijson)
endif()
