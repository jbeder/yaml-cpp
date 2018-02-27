include(ExternalProject)

ExternalProject_Add(
	googletest_project
	SOURCE_DIR "${CMAKE_SOURCE_DIR}/test/gtest-1.8.0"
	INSTALL_DIR "${CMAKE_BINARY_DIR}/prefix"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DBUILD_GMOCK=ON
)

add_library(gmock UNKNOWN IMPORTED)
set_target_properties(gmock PROPERTIES
	IMPORTED_LOCATION ${PROJECT_BINARY_DIR}/prefix/lib/libgmock.a
)

find_package(Threads)

include_directories(SYSTEM "${PROJECT_BINARY_DIR}/prefix/include")

set(gtest_force_shared_crt ${MSVC_SHARED_RT} CACHE BOOL
  "Use shared (DLL) run-time lib even when Google Test built as a static lib.")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
   CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(yaml_test_flags "-Wno-variadic-macros -Wno-sign-compare")

	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		set(yaml_test_flags "${yaml_test_flags} -Wno-c99-extensions")
	endif()
endif()

file(GLOB test_headers [a-z_]*.h)
file(GLOB test_sources [a-z_]*.cpp integration/[a-z_]*.cpp node/[a-z_]*.cpp)
file(GLOB test_new_api_sources new-api/[a-z]*.cpp)

list(APPEND test_sources ${test_new_api_sources})
add_sources(${test_sources} ${test_headers})

include_directories(${YAML_CPP_SOURCE_DIR}/test)

add_executable(run-tests
	${test_sources}
	${test_headers}
)

add_dependencies(run-tests googletest_project)

set_target_properties(run-tests PROPERTIES
  COMPILE_FLAGS "${yaml_c_flags} ${yaml_cxx_flags} ${yaml_test_flags}"
)
target_link_libraries(run-tests
	yaml-cpp
	gmock
	${CMAKE_THREAD_LIBS_INIT})

add_test(yaml-test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/run-tests)
