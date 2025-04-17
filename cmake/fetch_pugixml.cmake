FetchContent_Declare(
  pugixml
  GIT_REPOSITORY https://github.com/zeux/pugixml
  GIT_TAG v1.15
)

if(NOT pugixml_POPULATED)
  FetchContent_Populate(pugixml)
endif()
