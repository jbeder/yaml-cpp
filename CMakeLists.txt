# 3.5 is actually available almost everywhere.
# 3.30 as the upper policy limit avoids CMake deprecation warnings.
cmake_minimum_required(VERSION 3.5...3.30)

# enable MSVC_RUNTIME_LIBRARY target property
# see https://cmake.org/cmake/help/latest/policy/CMP0091.html
if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

project(YAML_CPP VERSION 0.9.0 LANGUAGES CXX)

set(YAML_CPP_MAIN_PROJECT OFF)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(YAML_CPP_MAIN_PROJECT ON)
endif()

# Include required modules
include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)
include(GNUInstallDirs)
include(CTest)

# Project options
option(YAML_CPP_BUILD_CONTRIB "Enable yaml-cpp contrib in library" ON)
option(YAML_CPP_BUILD_TOOLS "Enable parse tools" ON)
option(YAML_BUILD_SHARED_LIBS "Build yaml-cpp shared library" ${BUILD_SHARED_LIBS})
option(YAML_CPP_INSTALL "Enable generation of yaml-cpp install targets" ${YAML_CPP_MAIN_PROJECT})
option(YAML_CPP_FORMAT_SOURCE "Format source" ${YAML_CPP_MAIN_PROJECT})
option(YAML_CPP_DISABLE_UNINSTALL  "Disable uninstallation of yaml-cpp" OFF)
option(YAML_USE_SYSTEM_GTEST "Use system googletest if found" OFF)
option(YAML_ENABLE_PIC "Use Position-Independent Code " ON)

# Dependent options
cmake_dependent_option(YAML_CPP_BUILD_TESTS
  "Enable yaml-cpp tests" OFF
  "BUILD_TESTING;YAML_CPP_MAIN_PROJECT" OFF)
cmake_dependent_option(YAML_MSVC_SHARED_RT
  "MSVC: Build yaml-cpp with shared runtime libs (/MD)" ON
  "CMAKE_SYSTEM_NAME MATCHES Windows" OFF)

# Installation paths
set(YAML_CPP_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/yaml-cpp"
  CACHE STRING "Path to install the CMake package to")

# Find clang-format if formatting is enabled
if (YAML_CPP_FORMAT_SOURCE)
    find_program(YAML_CPP_CLANG_FORMAT_EXE NAMES clang-format)
endif()

# Set shared/static label
if (YAML_BUILD_SHARED_LIBS)
  set(yaml-cpp-label-postfix "shared")
else()
  set(yaml-cpp-label-postfix "static")
endif()

# Set up compiler flags and variables
set(build-shared $<BOOL:${YAML_BUILD_SHARED_LIBS}>)
set(build-windows-dll $<AND:$<BOOL:${CMAKE_HOST_WIN32}>,${build-shared}>)
set(not-msvc $<NOT:$<CXX_COMPILER_ID:MSVC>>)
set(msvc-shared_rt $<BOOL:${YAML_MSVC_SHARED_RT}>)

# Set MSVC runtime library
if (NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
  set(CMAKE_MSVC_RUNTIME_LIBRARY
    MultiThreaded$<$<CONFIG:Debug>:Debug>$<${msvc-shared_rt}:DLL>)
endif()

# Define MSVC runtime variables for use in subdirectories
set(msvc-rt $<TARGET_PROPERTY:MSVC_RUNTIME_LIBRARY>)
set(msvc-rt-mtd-static $<STREQUAL:${msvc-rt},MultiThreadedDebug>)
set(msvc-rt-mt-static $<STREQUAL:${msvc-rt},MultiThreaded>)
set(msvc-rt-mtd-dll $<STREQUAL:${msvc-rt},MultiThreadedDebugDLL>)
set(msvc-rt-mt-dll $<STREQUAL:${msvc-rt},MultiThreadedDLL>)
set(backport-msvc-runtime $<VERSION_LESS:${CMAKE_VERSION},3.15>)

# Set debug postfix
if (NOT DEFINED CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX "d")
endif()

# Build the library
add_subdirectory(src)

# Set version properties
set_target_properties(yaml-cpp PROPERTIES
  VERSION "${PROJECT_VERSION}"
  SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
  PROJECT_LABEL "yaml-cpp ${yaml-cpp-label-postfix}"
  DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

# Configure package files
set(EXPORT_TARGETS yaml-cpp::yaml-cpp)
configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/yaml-cpp-config.cmake.in"
  "${PROJECT_BINARY_DIR}/yaml-cpp-config.cmake"
  INSTALL_DESTINATION "${YAML_CPP_INSTALL_CMAKEDIR}"
  PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR)
unset(EXPORT_TARGETS)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/yaml-cpp-config-version.cmake"
  COMPATIBILITY AnyNewerVersion)

configure_file(yaml-cpp.pc.in yaml-cpp.pc @ONLY)

# Installation
if (YAML_CPP_INSTALL)
  install(TARGETS yaml-cpp
    EXPORT yaml-cpp-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
                FILES_MATCHING PATTERN "*.h")
  install(EXPORT yaml-cpp-targets
    NAMESPACE yaml-cpp::
    DESTINATION "${YAML_CPP_INSTALL_CMAKEDIR}")
  install(FILES
      "${PROJECT_BINARY_DIR}/yaml-cpp-config.cmake"
      "${PROJECT_BINARY_DIR}/yaml-cpp-config-version.cmake"
    DESTINATION "${YAML_CPP_INSTALL_CMAKEDIR}")
  install(FILES "${PROJECT_BINARY_DIR}/yaml-cpp.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

# Add tests if enabled
if(YAML_CPP_BUILD_TESTS)
  add_subdirectory(test)
endif()

# Add tools if enabled
if(YAML_CPP_BUILD_TOOLS)
  add_subdirectory(util)
endif()

# Add formatting target if enabled
if (YAML_CPP_FORMAT_SOURCE AND YAML_CPP_CLANG_FORMAT_EXE)
  add_custom_target(format
    COMMAND clang-format --style=file -i $<TARGET_PROPERTY:yaml-cpp,SOURCES>
    COMMAND_EXPAND_LISTS
    COMMENT "Running clang-format"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    VERBATIM)
endif()

# Add uninstall target
if(YAML_CPP_INSTALL AND NOT YAML_CPP_DISABLE_UNINSTALL AND NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()