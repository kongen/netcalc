ACLOCAL_AMFLAGS=-I m4
AUTOMAKE_OPTIONS = foreign

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnetcalc.pc

EXTRA_DIST = libnetcalc.pc.in README.md LICENSE .github/workflows/ci.yml

SUBDIRS = src
if BUILD_TESTS
SUBDIRS += tests
endif

.PHONY: dist-check release

dist-check: distcheck

release: distcheck
