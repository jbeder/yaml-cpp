AC_INIT(yaml-cpp, 0.6.2)
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign 1.10])
AM_MAINTAINER_MODE
LT_INIT

AC_ARG_WITH(boostinc,
	AC_HELP_STRING([--with-boostinc], [where to find boost headers (path w/o trailing /boost)])
)

AC_PROG_INSTALL
AC_PROG_CXX

#AC_CONFIG_SUBDIRS([src include])

AM_CONDITIONAL([BOOSTINC], [test -n "$with_boostinc"])
AC_SUBST([boostinc],"$with_boostinc")

AC_CONFIG_FILES(Makefile src/Makefile include/Makefile)
AC_OUTPUT
