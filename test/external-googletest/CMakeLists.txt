
# Note:
# yaml-cpp as of 0fdb1b910c1a31db80b7521446166d06cad10813
# would include gmock-1.7.0 (apparently) full source code in
# $(git rev-parse --show-toplevel)/test/gmock-1.7.0
#
# However, I (@keith-bennett-gbg) encountered an issue apparently related
# to the version of gmock. To solve, I simply grabbed whatever version
# 'master' was at the time, aa148eb2b7f70ede0eb10de34b6254826bfb34f4
#
# However, the way this whole thing is implemented is rather a code smell.
# Specifically, gmock and gtest are also CMake projects, so we should not
# be using add_subdirectory() here. The correct way to achieve the desired
# behavior is lightly described in cmake's mailing list archive:
# http://public.kitware.com/pipermail/cmake/2015-August/061369.html
#
# Searching the web for 'cmake superproject' should reveal what
# needs to occur here.
#
# Since this project has already incurred the technical debt,
# I'll leave it as is. However, note that this does already
# interfere (not too terribly) with some of my own projects that
# *also* use gtest.
#

set(gtest_force_shared_crt ${MSVC_SHARED_RT} CACHE BOOL
  "Use shared (DLL) run-time lib even when Google Test built as a static lib.")

add_subdirectory(googletest)

