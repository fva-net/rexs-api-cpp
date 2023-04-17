configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Version.hxx.in"
  "${PROJECT_BINARY_DIR}/rexsapi/Version.hxx"
)

add_library(rexsapi INTERFACE)

target_sources(rexsapi INTERFACE
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Rexsapi.hxx

  ${PROJECT_SOURCE_DIR}/include/rexsapi/Attribute.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Base64.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/CodedValue.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Component.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/ConversionHelper.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Defines.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Exception.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/FileTypes.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/FileUtils.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Format.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Json.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/JsonModelLoader.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/JsonModelSerializer.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/JsonSchemaValidator.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/JsonSerializer.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/JsonValueDecoder.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/LoadSpectrum.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Mode.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Model.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/ModelBuilder.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/ModelHelper.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/ModelLoader.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/ModelSaver.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/ModelVisitor.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Relation.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/RelationTypeChecker.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Result.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/RexsVersion.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Types.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Unit.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/ValidityChecker.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Value_Details.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Value.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/Xml.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/XMLModelLoader.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/XMLModelSerializer.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/XMLSerializer.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/XmlUtils.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/XMLValueDecoder.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/XSDSchemaValidator.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/ZipArchive.hxx

  ${PROJECT_SOURCE_DIR}/include/rexsapi/database/Attribute.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/database/Component.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/database/ComponentAttributeMapper.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/database/EnumValues.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/database/FileResourceLoader.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/database/Interval.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/database/Model.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/database/ModelRegistry.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/database/Unit.hxx
  ${PROJECT_SOURCE_DIR}/include/rexsapi/database/XMLModelLoader.hxx
)

target_include_directories(rexsapi INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>)
target_include_directories(rexsapi SYSTEM INTERFACE "${date_SOURCE_DIR}/include")
target_include_directories(rexsapi SYSTEM INTERFACE "${fmt_SOURCE_DIR}/include")
target_include_directories(rexsapi SYSTEM INTERFACE "${json_SOURCE_DIR}/single_include")
target_include_directories(rexsapi SYSTEM INTERFACE "${pugixml_SOURCE_DIR}/src")
target_include_directories(rexsapi SYSTEM INTERFACE "${valijson_SOURCE_DIR}/include")
target_include_directories(rexsapi INTERFACE ${PROJECT_BINARY_DIR})
target_link_libraries(rexsapi INTERFACE libs::miniz)
target_compile_options(rexsapi INTERFACE ${REXSAPI_COMPILE_OPTIONS})

if(REXSAPI_MASTER_PROJECT)
  install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include"
    DESTINATION .
  )
  install(
    DIRECTORY "${PROJECT_BINARY_DIR}/rexsapi"
    DESTINATION ./include
  )
  install(
    DIRECTORY "${fmt_SOURCE_DIR}/include" "${valijson_SOURCE_DIR}/include"
    DESTINATION ./deps
  )
  install(
    DIRECTORY "${json_SOURCE_DIR}/single_include/" "${pugixml_SOURCE_DIR}/src/"
    DESTINATION ./deps/include
  )
  install(
    DIRECTORY "${miniz_BINARY_DIR}/miniz"
    DESTINATION ./deps/include
  )
  install(
    DIRECTORY "${date_SOURCE_DIR}/include"
    DESTINATION ./deps
  )
  install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/examples"
    DESTINATION .
    FILES_MATCHING PATTERN "*.cxx"
  )
  install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/models"
    DESTINATION .
  )
  install(
    DIRECTORY "${PROJECT_BINARY_DIR}/html"
    DESTINATION .
  )

  set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${PLATFORM_NAME}-${PROJECT_VERSION}")
  set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
  set(CPACK_GENERATOR "ZIP")
  set(CPACK_PACKAGE_CHECKSUM "SHA256")
  include(CPack)
endif()
