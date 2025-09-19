# yaml-cpp contrib directory

# Contrib source files
set(YAML_CPP_CONTRIB_SOURCES
    graphbuilder.cpp
    graphbuilderadapter.cpp
)

# Add contrib sources to the main target if enabled
if(YAML_CPP_BUILD_CONTRIB)
  target_sources(yaml-cpp
    PRIVATE
      ${YAML_CPP_CONTRIB_SOURCES})
endif()
