
find_package(Doxygen)

if(BUILD_WITH_DOCS AND DOXYGEN_FOUND)
  message(STATUS "Building with documentation")

  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md")
  set(DOXYGEN_EXTRACT_STATIC YES)
  set(DOXYGEN_EXTRACT_ALL YES)
  doxygen_add_docs(docs
    ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/README.md
  )
endif()
