if(NOT DEFINED YAML_CPP_PC_FILE)
  message(FATAL_ERROR "YAML_CPP_PC_FILE is required")
endif()

file(READ "${YAML_CPP_PC_FILE}" yaml_cpp_pc)
set(static_define "Cflags.private: -DYAML_CPP_STATIC_DEFINE")
string(FIND "${yaml_cpp_pc}" "${static_define}" static_define_pos)

if(YAML_BUILD_SHARED_LIBS)
  if(NOT static_define_pos EQUAL -1)
    message(FATAL_ERROR
      "Shared-library pkg-config metadata contains ${static_define}")
  endif()
elseif(static_define_pos EQUAL -1)
  message(FATAL_ERROR
    "Static-library pkg-config metadata does not contain ${static_define}")
endif()
