find_package(Threads REQUIRED)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_MOCK ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

if(YAML_USE_SYSTEM_GTEST)
  find_package(GTest)
  if (NOT GTEST_FOUND)
    message(FATAL_ERROR "system googletest was requested but not found")
  endif()
else()
    add_subdirectory(
      "${CMAKE_CURRENT_SOURCE_DIR}/googletest-1.16.0"
      "${CMAKE_CURRENT_BINARY_DIR}/prefix")
    include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/googletest-1.16.0/googletest/include")
endif()

file(GLOB test-new-api-sources CONFIGURE_DEPENDS "new-api/*.cpp")
file(GLOB test-sources CONFIGURE_DEPENDS "*.cpp" "integration/*.cpp" "node/*.cpp")

add_executable(yaml-cpp-tests "")
target_sources(yaml-cpp-tests
  PRIVATE
    ${test-new-api-sources}
    ${test-sources})
target_include_directories(yaml-cpp-tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/integration
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src)
target_compile_options(yaml-cpp-tests
  PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:-Wno-c99-extensions -Wno-variadic-macros -Wno-sign-compare>
    $<$<CXX_COMPILER_ID:GNU>:-Wno-variadic-macros -Wno-sign-compare -Wno-narrowing>)
target_link_libraries(yaml-cpp-tests
  PRIVATE
    Threads::Threads
    yaml-cpp
    gtest
    gmock)

set_property(TARGET yaml-cpp-tests PROPERTY CXX_STANDARD_REQUIRED ON)
if (NOT DEFINED CMAKE_CXX_STANDARD)
  set_target_properties(yaml-cpp-tests PROPERTIES CXX_STANDARD 11)
endif()


add_test(NAME yaml-cpp::test COMMAND yaml-cpp-tests)

if (build-windows-dll)
  add_custom_command(
    TARGET yaml-cpp-tests
    POST_BUILD COMMAND ${CMAKE_COMMAND} -E
    copy_if_different "$<TARGET_FILE:yaml-cpp>" "$<TARGET_FILE_DIR:yaml-cpp-tests>")
endif()
