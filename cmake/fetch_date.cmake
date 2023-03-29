FetchContent_Declare(
  date
  URL https://github.com/HowardHinnant/date/archive/refs/tags/v3.0.1.tar.gz
)

FetchContent_GetProperties(date)

if(NOT date_POPULATED)
  FetchContent_Populate(date)
endif()
