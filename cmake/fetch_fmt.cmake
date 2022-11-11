FetchContent_Declare(
  fmt
  URL https://github.com/fmtlib/fmt/archive/refs/tags/8.1.1.tar.gz
)

FetchContent_GetProperties(fmt)

if(NOT fmt_POPULATED)
  FetchContent_Populate(fmt)
endif()
