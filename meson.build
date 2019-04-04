project('yaml-cpp', 'cpp',
  version : '0.6.2',
  meson_version : '>=0.46.0',
  license : 'MIT')

sources = files([
  'src/binary.cpp',
  'src/convert.cpp',
  'src/directives.cpp',
  'src/emit.cpp',
  'src/emitfromevents.cpp',
  'src/emitter.cpp',
  'src/emitterstate.cpp',
  'src/emitterutils.cpp',
  'src/exceptions.cpp',
  'src/exp.cpp',
  'src/memory.cpp',
  'src/nodebuilder.cpp',
  'src/node.cpp',
  'src/node_data.cpp',
  'src/nodeevents.cpp',
  'src/null.cpp',
  'src/ostream_wrapper.cpp',
  'src/parse.cpp',
  'src/parser.cpp',
  'src/regex_yaml.cpp',
  'src/scanner.cpp',
  'src/scanscalar.cpp',
  'src/scantag.cpp',
  'src/scantoken.cpp',
  'src/simplekey.cpp',
  'src/singledocparser.cpp',
  'src/stream.cpp',
  'src/tag.cpp',
])

public_headers = files([
  'include/yaml-cpp/anchor.h',
  'include/yaml-cpp/binary.h',
  'include/yaml-cpp/dll.h',
  'include/yaml-cpp/emitfromevents.h',
  'include/yaml-cpp/emitterdef.h',
  'include/yaml-cpp/emitter.h',
  'include/yaml-cpp/emittermanip.h',
  'include/yaml-cpp/emitterstyle.h',
  'include/yaml-cpp/eventhandler.h',
  'include/yaml-cpp/exceptions.h',
  'include/yaml-cpp/mark.h',
  'include/yaml-cpp/null.h',
  'include/yaml-cpp/ostream_wrapper.h',
  'include/yaml-cpp/parser.h',
  'include/yaml-cpp/stlemitter.h',
  'include/yaml-cpp/traits.h',
  'include/yaml-cpp/yaml.h',
])

public_contrib_headers = files([
  'include/yaml-cpp/contrib/anchordict.h',
  'include/yaml-cpp/contrib/graphbuilder.h',
])

public_node_headers = files([
  'include/yaml-cpp/node/convert.h',
  'include/yaml-cpp/node/emit.h',
  'include/yaml-cpp/node/impl.h',
  'include/yaml-cpp/node/iterator.h',
  'include/yaml-cpp/node/node.h',
  'include/yaml-cpp/node/parse.h',
  'include/yaml-cpp/node/ptr.h',
  'include/yaml-cpp/node/type.h',
])

public_node_detail_headers = files([
  'include/yaml-cpp/node/detail/bool_type.h',
  'include/yaml-cpp/node/detail/impl.h',
  'include/yaml-cpp/node/detail/iterator_fwd.h',
  'include/yaml-cpp/node/detail/iterator.h',
  'include/yaml-cpp/node/detail/memory.h',
  'include/yaml-cpp/node/detail/node_data.h',
  'include/yaml-cpp/node/detail/node.h',
  'include/yaml-cpp/node/detail/node_iterator.h',
  'include/yaml-cpp/node/detail/node_ref.h',
])

should_install = not meson.is_subproject()

libyamlcpp = library('yaml-cpp', sources,
  include_directories : 'include',
  version : meson.project_version(),
  install : should_install)

if should_install
  install_headers(public_headers, subdir : 'yaml-cpp')
  install_headers(public_contrib_headers, subdir : 'yaml-cpp/contrib')
  install_headers(public_node_headers, subdir : 'yaml-cpp/node')
  install_headers(public_node_detail_headers, subdir : 'yaml-cpp/node/detail')

  pkg = import('pkgconfig')
  pkg.generate(libyamlcpp,
    name : 'libyaml-cpp',
    description : 'A YAML parser and emitter in C++',
  )

  # TODO: it seems possible to generate cmake packages using meson
  #       but I do not know how to generate yaml-cpp-targets.cmake
  # cmake = import('cmake')
  # conf = configuration_data()
  # conf.set('CONFIG_INCLUDE_DIRS', '${YAML_CPP_CMAKE_DIR}/../../../include')
  # conf.set('EXPORT_TARGETS', 'yaml-cpp')

  # cmake.configure_package_config_file(
  #   name : 'yaml-cpp',
  #   input : 'yaml-cpp-config.cmake.in',
  #   configuration : conf,
  # )
  # 
  # cmake.write_basic_package_version_file(
  #   name : 'yaml-cpp',
  #   version : meson.project_version(),
  # )
endif

yamlcpp_dep = declare_dependency(
  include_directories : 'include',
  link_with : libyamlcpp,
  version : meson.project_version(),
)

gtest_dep = dependency('gtest', fallback : ['gtest', 'gtest_dep'], required : false)
gmock_dep = dependency('gmock', fallback : ['gtest', 'gmock_dep'], required : false)
if gtest_dep.found()
  subdir('test')
  test('all tests', tests)
endif
