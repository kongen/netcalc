cmake_minimum_required(VERSION 3.16)

project(netcalc VERSION 1.0.0 LANGUAGES CXX)

include(CheckCXXCompilerFlag)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_SHARED_LIBS "Build libraries as shared objects" ON)

set(NETCALC_BUILD_VERSION "")
if(DEFINED ENV{NETCALC_VERSION} AND NOT "$ENV{NETCALC_VERSION}" STREQUAL "")
	set(NETCALC_BUILD_VERSION "$ENV{NETCALC_VERSION}")
elseif(DEFINED ENV{GITHUB_REF_NAME} AND NOT "$ENV{GITHUB_REF_NAME}" STREQUAL "")
	set(NETCALC_BUILD_VERSION "$ENV{GITHUB_REF_NAME}")
elseif(DEFINED ENV{GITHUB_SHA} AND NOT "$ENV{GITHUB_SHA}" STREQUAL "")
	string(SUBSTRING "$ENV{GITHUB_SHA}" 0 7 NETCALC_BUILD_VERSION)
else()
	execute_process(
		COMMAND git rev-parse --short HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE NETCALC_BUILD_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
	)
endif()
if(NETCALC_BUILD_VERSION STREQUAL "")
	set(NETCALC_BUILD_VERSION "${PROJECT_VERSION}")
endif()
set(NETCALC_PC_VERSION "${PROJECT_VERSION}")

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src)
configure_file(
	${CMAKE_SOURCE_DIR}/src/netcalc_build_config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/src/netcalc_build_config.h
	@ONLY
)
configure_file(
	${CMAKE_SOURCE_DIR}/libnetcalc.pc.in
	${CMAKE_CURRENT_BINARY_DIR}/libnetcalc.pc
	@ONLY
)

add_executable(ipv4calc
	src/ipv4calc.cpp
	src/main.cpp
)

set(NETCALC_LIBRARY_SOURCES
	src/netcalc.cpp
	src/netcalc_format.cpp
)

set(NETCALC_PUBLIC_HEADERS
	src/netcalc.h
	src/netcalc_format.h
	src/ipv4calc.h
	src/ipv6calc.h
)

add_library(netcalc
	${NETCALC_LIBRARY_SOURCES}
	${NETCALC_PUBLIC_HEADERS}
)
set_target_properties(netcalc PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
)
if(APPLE)
	set_target_properties(netcalc PROPERTIES
		INSTALL_NAME_DIR ${CMAKE_INSTALL_FULL_LIBDIR}
	)
endif()

target_include_directories(netcalc
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
		$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(netcalc PUBLIC cxx_std_11)

target_include_directories(ipv4calc PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_SOURCE_DIR}/src)
target_compile_features(ipv4calc PRIVATE cxx_std_11)
target_link_libraries(ipv4calc PRIVATE netcalc)

add_executable(ipv6calc
	src/ipv6calc.cpp
	src/ipv6_main.cpp
)

target_include_directories(ipv6calc PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_SOURCE_DIR}/src)
target_compile_features(ipv6calc PRIVATE cxx_std_11)
target_link_libraries(ipv6calc PRIVATE netcalc)

if(UNIX)
	set_target_properties(ipv4calc ipv6calc PROPERTIES
		INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}
	)
endif()

install(TARGETS netcalc ipv4calc ipv6calc
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
	${NETCALC_PUBLIC_HEADERS}
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/libnetcalc.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

include(CTest)

if(BUILD_TESTING)
	check_cxx_compiler_flag("-std=c++17" HAVE_CXX17_FLAG)
	if(HAVE_CXX17_FLAG)
		find_package(GTest QUIET CONFIG)
		if(GTest_FOUND)
			add_subdirectory(tests)
		else()
			message(STATUS "GTest not found; unit tests will not be built")
		endif()
	else()
		message(STATUS "C++17 compiler support not found; unit tests will not be built")
	endif()
endif()
