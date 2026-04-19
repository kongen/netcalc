m4_define([netcalc_major_version], [1])
m4_define([netcalc_minor_version], [0])
m4_define([netcalc_micro_version], [0])
m4_define([netcalc_version],[netcalc_major_version.netcalc_minor_version.netcalc_micro_version])

AC_INIT([netcalc],[netcalc_version],[shm@hallas.nu])
AM_INIT_AUTOMAKE([-Wall -Werror dist-xz foreign subdir-objects])

AC_PROG_CXX
AC_PROG_LN_S
AM_PROG_AR
LT_INIT
PKG_PROG_PKG_CONFIG

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_CHECK_FUNCS([inet_ntoa])
AC_CHECK_HEADERS([arpa/inet.h])

AX_CXX_COMPILE_STDCXX([11],[noext],[mandatory])

AC_ARG_VAR([NETCALC_VERSION], [Version string override for build metadata])

build_version="$NETCALC_VERSION"
if test -z "$build_version"; then
  if test -n "$GITHUB_REF_NAME"; then
    build_version="$GITHUB_REF_NAME"
  elif test -n "$GITHUB_SHA"; then
    build_version=`printf '%s' "$GITHUB_SHA" | cut -c1-7`
  else
    build_version=`git -C "$srcdir" rev-parse --short HEAD 2>/dev/null || true`
  fi
fi
if test -z "$build_version"; then
  build_version="$PACKAGE_VERSION"
fi
AC_SUBST([NETCALC_BUILD_VERSION], [$build_version])
AC_SUBST([NETCALC_PC_VERSION], [$PACKAGE_VERSION])

CXX17_CXXFLAGS=""
build_tests=no

AC_CACHE_CHECK([whether $CXX supports C++17 with -std=c++17],
 [ac_cv_cxx17_flag],
 [AC_LANG_PUSH([C++])
  ac_save_CXX="$CXX"
  CXX="$CXX -std=c++17"
  AC_COMPILE_IFELSE(
   [AC_LANG_SOURCE([[
     #if __cplusplus < 201703L
     #error "C++17 support is required for tests"
     #endif
     int main() { return 0; }
   ]])],
   [ac_cv_cxx17_flag=yes],
   [ac_cv_cxx17_flag=no])
  CXX="$ac_save_CXX"
  AC_LANG_POP([C++])])

if test "x$ac_cv_cxx17_flag" = "xyes"; then
  CXX17_CXXFLAGS="-std=c++17"
  PKG_CHECK_MODULES([GTEST], [gmock_main], [build_tests=yes], [build_tests=no])
  if test "x$build_tests" = "xno"; then
    AC_MSG_NOTICE([googletest not found; unit tests will not be built])
  fi
else
  AC_MSG_NOTICE([C++17 compiler support not found; unit tests will not be built])
fi
AC_SUBST([CXX17_CXXFLAGS])
AM_CONDITIONAL([BUILD_TESTS], [test "x$build_tests" = "xyes"])

AC_CONFIG_FILES([
 Makefile
 libnetcalc.pc
 src/Makefile
 src/netcalc_build_config.h
 tests/Makefile
])
AC_OUTPUT
