include("${YAML_CPP_SOURCE_DIR}/cmake/yaml-cpp-sources.cmake")

file(GLOB actual_sources
  LIST_DIRECTORIES false
  RELATIVE "${YAML_CPP_SOURCE_DIR}"
  "${YAML_CPP_SOURCE_DIR}/src/*.cpp"
  "${YAML_CPP_SOURCE_DIR}/src/contrib/*.cpp")

set(expected_sources
  ${yaml-cpp-contrib-sources}
  ${yaml-cpp-sources})

list(SORT actual_sources)
list(SORT expected_sources)

if(NOT "${actual_sources}" STREQUAL "${expected_sources}")
  message(FATAL_ERROR
    "yaml-cpp source list is out of date.\n"
    "Expected: ${expected_sources}\n"
    "Actual: ${actual_sources}")
endif()
