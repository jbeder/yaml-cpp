test_sources = files([
  'main.cpp',
  'ostream_wrapper_test.cpp',
  'regex_test.cpp',
  'node/node_test.cpp',
  'integration/emitter_test.cpp', 
  'integration/encoding_test.cpp', 
  'integration/gen_emitter_test.cpp', 
  'integration/handler_spec_test.cpp', 
  'integration/handler_test.cpp', 
  'integration/load_node_test.cpp', 
  'integration/node_spec_test.cpp', 
]) 

test_dep = yamlcpp_dep

tests = executable('tests', test_sources,
  dependencies : [ gtest_dep, gmock_dep, test_dep ],
  include_directories : [ '../src' ])
