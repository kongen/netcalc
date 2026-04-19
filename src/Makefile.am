AM_CPPFLAGS = -I$(builddir) -I$(srcdir)
AM_CXXFLAGS = -Wall -Werror

lib_LTLIBRARIES = libnetcalc.la
libnetcalc_la_SOURCES = netcalc.cpp netcalc.h netcalc_format.cpp netcalc_format.h
libnetcalc_la_LDFLAGS = -version-info 1:0:0

include_HEADERS = netcalc.h netcalc_format.h ipv4calc.h ipv6calc.h

bin_PROGRAMS = ipv4calc ipv6calc
ipv4calc_SOURCES = main.cpp ipv4calc.cpp ipv4calc.h
ipv4calc_LDADD = libnetcalc.la
ipv6calc_SOURCES = ipv6_main.cpp ipv6calc.cpp ipv6calc.h
ipv6calc_LDADD = libnetcalc.la

BUILT_SOURCES = netcalc_build_config.h
EXTRA_DIST = netcalc_build_config.h.in
CLEANFILES = netcalc_build_config.h
