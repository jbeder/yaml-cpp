# yaml-cpp source directory

# Core source files
set(YAML_CPP_SOURCES
    binary.cpp
    convert.cpp
    depthguard.cpp
    directives.cpp
    emit.cpp
    emitfromevents.cpp
    emitter.cpp
    emitterstate.cpp
    emitterutils.cpp
    exceptions.cpp
    exp.cpp
    fptostring.cpp
    memory.cpp
    node.cpp
    node_data.cpp
    nodebuilder.cpp
    nodeevents.cpp
    null.cpp
    ostream_wrapper.cpp
    parse.cpp
    parser.cpp
    regex_yaml.cpp
    scanner.cpp
    scanscalar.cpp
    scantag.cpp
    scantoken.cpp
    simplekey.cpp
    singledocparser.cpp
    stream.cpp
    tag.cpp
)

# Main library definition
if (YAML_BUILD_SHARED_LIBS)
  set(yaml-cpp-type SHARED)
else()
  set(yaml-cpp-type STATIC)
endif()

# Create library target
add_library(yaml-cpp ${yaml-cpp-type} "")
add_library(yaml-cpp::yaml-cpp ALIAS yaml-cpp)

# Set library properties
set_property(TARGET yaml-cpp
  PROPERTY
    MSVC_RUNTIME_LIBRARY ${CMAKE_MSVC_RUNTIME_LIBRARY})
set_property(TARGET yaml-cpp
  PROPERTY
    CXX_STANDARD_REQUIRED ON)

if (NOT YAML_BUILD_SHARED_LIBS)
    set_property(TARGET yaml-cpp PROPERTY POSITION_INDEPENDENT_CODE ${YAML_ENABLE_PIC})
endif()

# Add include directories
target_include_directories(yaml-cpp
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)

# Set C++ standard
if (NOT DEFINED CMAKE_CXX_STANDARD)
  set_target_properties(yaml-cpp
    PROPERTIES
      CXX_STANDARD 11)
endif()

# Add compile options and definitions
target_compile_definitions(yaml-cpp
  PUBLIC
    $<$<NOT:$<BOOL:${YAML_BUILD_SHARED_LIBS}>>:YAML_CPP_STATIC_DEFINE>
  PRIVATE
    $<${build-windows-dll}:${PROJECT_NAME}_DLL>
    $<$<NOT:$<BOOL:${YAML_CPP_BUILD_CONTRIB}>>:YAML_CPP_NO_CONTRIB>)

# Add sources to the target
target_sources(yaml-cpp
  PRIVATE
    ${YAML_CPP_SOURCES})

# Add contrib directory
add_subdirectory(contrib)
