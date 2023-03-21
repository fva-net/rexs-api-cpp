FetchContent_Declare(
  fmt
  URL https://github.com/fmtlib/fmt/archive/refs/tags/9.1.0.tar.gz
)

FetchContent_GetProperties(fmt)

if(NOT fmt_POPULATED)
  FetchContent_Populate(fmt)
endif()
