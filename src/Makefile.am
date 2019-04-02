AUTOMAKE_OPTIONS=foreign subdir-objects
#ACLOCAL_AMFLAGS=-I@srcdir@/../m4

AM_CPPFLAGS=-I@srcdir@/../include

if BOOSTINC
AM_CPPFLAGS+=-I@boostinc@
endif

lib_LTLIBRARIES=libyaml-cpp.la

include_HEADERS=

libyaml_cpp_la_SOURCES=emitter.cpp
libyaml_cpp_la_SOURCES+=scantoken.cpp
libyaml_cpp_la_SOURCES+=emitterstate.cpp
libyaml_cpp_la_SOURCES+=node.cpp
libyaml_cpp_la_SOURCES+=convert.cpp
libyaml_cpp_la_SOURCES+=parse.cpp
libyaml_cpp_la_SOURCES+=directives.cpp
libyaml_cpp_la_SOURCES+=parser.cpp
libyaml_cpp_la_SOURCES+=node_data.cpp
libyaml_cpp_la_SOURCES+=memory.cpp
libyaml_cpp_la_SOURCES+=tag.cpp
libyaml_cpp_la_SOURCES+=exceptions.cpp
libyaml_cpp_la_SOURCES+=emitfromevents.cpp
libyaml_cpp_la_SOURCES+=scanner.cpp
libyaml_cpp_la_SOURCES+=binary.cpp
libyaml_cpp_la_SOURCES+=contrib/graphbuilderadapter.cpp
libyaml_cpp_la_SOURCES+=contrib/graphbuilder.cpp
libyaml_cpp_la_SOURCES+=exp.cpp
libyaml_cpp_la_SOURCES+=nodeevents.cpp
libyaml_cpp_la_SOURCES+=stream.cpp
libyaml_cpp_la_SOURCES+=scantag.cpp
libyaml_cpp_la_SOURCES+=singledocparser.cpp
libyaml_cpp_la_SOURCES+=nodebuilder.cpp
libyaml_cpp_la_SOURCES+=emitterutils.cpp
libyaml_cpp_la_SOURCES+=simplekey.cpp
libyaml_cpp_la_SOURCES+=null.cpp
libyaml_cpp_la_SOURCES+=regex_yaml.cpp
libyaml_cpp_la_SOURCES+=ostream_wrapper.cpp
libyaml_cpp_la_SOURCES+=scanscalar.cpp
libyaml_cpp_la_SOURCES+=emit.cpp
