FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest
  GIT_TAG v2.4.10
)

FetchContent_GetProperties(doctest)

if(NOT doctest_POPULATED)
  set(DOCTEST_NO_INSTALL ON)
  FetchContent_Populate(doctest)
endif()
