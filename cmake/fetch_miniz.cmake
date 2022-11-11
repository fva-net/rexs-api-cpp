FetchContent_Declare(
  miniz
  URL https://github.com/richgel999/miniz/archive/refs/tags/2.2.0.tar.gz
)

FetchContent_GetProperties(miniz)

if(NOT miniz_POPULATED)
  FetchContent_Populate(miniz)

  file(COPY ${miniz_SOURCE_DIR}/miniz.h DESTINATION ${miniz_BINARY_DIR}/miniz/)
  file(READ ${miniz_SOURCE_DIR}/miniz.h MINIZ_H)
  file(READ ${miniz_SOURCE_DIR}/miniz_common.h MINIZ_COMMON_H)
  file(READ ${miniz_SOURCE_DIR}/miniz_tdef.h MINIZ_TDEF_H)
  file(READ ${miniz_SOURCE_DIR}/miniz_tinfl.h MINIZ_TINFL_H)
  file(READ ${miniz_SOURCE_DIR}/miniz_zip.h MINIZ_ZIP_H)
  file(APPEND ${miniz_BINARY_DIR}/miniz/miniz.h
    "${MINIZ_COMMON_H} ${MINIZ_TDEF_H} ${MINIZ_TINFL_H} ${MINIZ_ZIP_H}")

  file(COPY ${miniz_SOURCE_DIR}/miniz.c DESTINATION ${miniz_BINARY_DIR}/miniz/)
  file(READ ${miniz_SOURCE_DIR}/miniz_tdef.c MINIZ_TDEF_C)
  file(READ ${miniz_SOURCE_DIR}/miniz_tinfl.c MINIZ_TINFL_C)
  file(READ ${miniz_SOURCE_DIR}/miniz_zip.c MINIZ_ZIP_C)
  file(APPEND ${miniz_BINARY_DIR}/miniz/miniz.c
    "${MINIZ_TDEF_C} ${MINIZ_TINFL_C} ${MINIZ_ZIP_C}")

  file(READ ${miniz_BINARY_DIR}/miniz/miniz.h AMAL_MINIZ_H)
  file(READ ${miniz_BINARY_DIR}/miniz/miniz.c AMAL_MINIZ_C)

  foreach(REPLACE_STRING miniz;miniz_common;miniz_tdef;miniz_tinfl;miniz_zip;miniz_export)
    string(REPLACE "#include \"${REPLACE_STRING}.h\"" "" AMAL_MINIZ_H "${AMAL_MINIZ_H}")
    string(REPLACE "#include \"${REPLACE_STRING}.h\"" "" AMAL_MINIZ_C "${AMAL_MINIZ_C}")
  endforeach()

  string(CONCAT AMAL_MINIZ_H "#define MINIZ_EXPORT\n" "${AMAL_MINIZ_H}")

  string(CONCAT AMAL_MINIZ_H "${AMAL_MINIZ_H}" "\n#ifndef MINIZ_HEADER_FILE_ONLY\n"
    "${AMAL_MINIZ_C}" "\n#endif // MINIZ_HEADER_FILE_ONLY\n")
  file(WRITE ${miniz_BINARY_DIR}/miniz/miniz.h "${AMAL_MINIZ_H}")
  file(REMOVE ${miniz_BINARY_DIR}/miniz/miniz.c)

  add_library(miniz INTERFACE)
  target_include_directories(miniz SYSTEM INTERFACE ${miniz_BINARY_DIR})
  add_library(libs::miniz ALIAS miniz)
endif()
