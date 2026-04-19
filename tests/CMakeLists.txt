find_package(GTest REQUIRED CONFIG)

add_executable(netcalc_tests
	test_netcalc.cpp
)

target_include_directories(netcalc_tests PRIVATE ${CMAKE_BINARY_DIR}/src ${CMAKE_SOURCE_DIR}/src)
target_compile_features(netcalc_tests PRIVATE cxx_std_17)
target_link_libraries(netcalc_tests PRIVATE netcalc GTest::gmock_main)

add_test(NAME netcalc_unit_tests COMMAND netcalc_tests)

add_executable(ipv4calc_tests
	test_ipv4calc.cpp
	${CMAKE_SOURCE_DIR}/src/ipv4calc.cpp
)

target_include_directories(ipv4calc_tests PRIVATE ${CMAKE_BINARY_DIR}/src ${CMAKE_SOURCE_DIR}/src)
target_compile_features(ipv4calc_tests PRIVATE cxx_std_17)
target_link_libraries(ipv4calc_tests PRIVATE netcalc GTest::gmock_main)

add_test(NAME ipv4calc_unit_tests COMMAND ipv4calc_tests)

add_executable(ipv4calc_cli_tests
	test_ipv4calc_cli.cpp
)

target_compile_features(ipv4calc_cli_tests PRIVATE cxx_std_17)
target_include_directories(ipv4calc_cli_tests PRIVATE ${CMAKE_BINARY_DIR}/src ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(ipv4calc_cli_tests PRIVATE netcalc GTest::gmock_main)

add_test(NAME ipv4calc_cli_tests COMMAND ipv4calc_cli_tests $<TARGET_FILE:ipv4calc>)

add_executable(ipv6calc_tests
	test_ipv6calc.cpp
	${CMAKE_SOURCE_DIR}/src/ipv6calc.cpp
)

target_include_directories(ipv6calc_tests PRIVATE ${CMAKE_BINARY_DIR}/src ${CMAKE_SOURCE_DIR}/src)
target_compile_features(ipv6calc_tests PRIVATE cxx_std_17)
target_link_libraries(ipv6calc_tests PRIVATE netcalc GTest::gmock_main)

add_test(NAME ipv6calc_unit_tests COMMAND ipv6calc_tests)

add_executable(ipv6calc_cli_tests
	test_ipv6calc_cli.cpp
)

target_compile_features(ipv6calc_cli_tests PRIVATE cxx_std_17)
target_include_directories(ipv6calc_cli_tests PRIVATE ${CMAKE_BINARY_DIR}/src ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(ipv6calc_cli_tests PRIVATE netcalc GTest::gmock_main)

add_test(NAME ipv6calc_cli_tests COMMAND ipv6calc_cli_tests $<TARGET_FILE:ipv6calc>)
