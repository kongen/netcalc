AM_CXXFLAGS = -Wall -Werror
AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_builddir)/src
AM_CPPFLAGS += @GTEST_CFLAGS@

check_PROGRAMS = test_netcalc test_ipv4calc test_ipv4calc_cli test_ipv6calc test_ipv6calc_cli
TESTS = $(check_PROGRAMS)

test_netcalc_SOURCES = test_netcalc.cpp
test_netcalc_CXXFLAGS = $(AM_CXXFLAGS) @CXX17_CXXFLAGS@
test_netcalc_LDADD = $(top_builddir)/src/libnetcalc.la @GTEST_LIBS@
test_ipv4calc_SOURCES = test_ipv4calc.cpp $(top_srcdir)/src/ipv4calc.cpp
test_ipv4calc_CXXFLAGS = $(AM_CXXFLAGS) @CXX17_CXXFLAGS@
test_ipv4calc_LDADD = $(top_builddir)/src/libnetcalc.la @GTEST_LIBS@
test_ipv4calc_cli_SOURCES = test_ipv4calc_cli.cpp
test_ipv4calc_cli_CXXFLAGS = $(AM_CXXFLAGS) @CXX17_CXXFLAGS@
test_ipv4calc_cli_LDADD = $(top_builddir)/src/libnetcalc.la @GTEST_LIBS@
test_ipv6calc_SOURCES = test_ipv6calc.cpp $(top_srcdir)/src/ipv6calc.cpp
test_ipv6calc_CXXFLAGS = $(AM_CXXFLAGS) @CXX17_CXXFLAGS@
test_ipv6calc_LDADD = $(top_builddir)/src/libnetcalc.la @GTEST_LIBS@
test_ipv6calc_cli_SOURCES = test_ipv6calc_cli.cpp
test_ipv6calc_cli_CXXFLAGS = $(AM_CXXFLAGS) @CXX17_CXXFLAGS@
test_ipv6calc_cli_LDADD = $(top_builddir)/src/libnetcalc.la @GTEST_LIBS@

TESTS_ENVIRONMENT = top_builddir="$(top_builddir)"
