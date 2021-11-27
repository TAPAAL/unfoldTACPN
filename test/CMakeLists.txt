find_package (Boost COMPONENTS unit_test_framework REQUIRED)
include_directories (${TEST_SOURCE_DIR}/src
                     ${Boost_INCLUDE_DIRS}
                     ${unfoldtacpn_SOURCE_DIR}
                     )
add_definitions (-DBOOST_TEST_DYN_LINK)

add_executable (old_format_test old_format_test.cpp)

target_link_libraries(old_format_test ${Boost_LIBRARIES} unfoldtacpn)

add_test(NAME old_format_test COMMAND old_format_test)
set_tests_properties(old_format_test PROPERTIES
    ENVIRONMENT TEST_FILES=${CMAKE_CURRENT_SOURCE_DIR}/old_format)

