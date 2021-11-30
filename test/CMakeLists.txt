find_package (Boost COMPONENTS unit_test_framework REQUIRED)
include_directories (${TEST_SOURCE_DIR}/src
                     ${Boost_INCLUDE_DIRS}
                     ${unfoldtacpn_SOURCE_DIR}
                     )
add_definitions (-DBOOST_TEST_DYN_LINK)

add_executable (old_format_test old_format_test.cpp)
add_executable (cpn_format_test cpn_format_test.cpp)
add_executable (string_query_test string_query_test.cpp)


target_link_libraries(old_format_test ${Boost_LIBRARIES} unfoldtacpn PetriParse Colored PQL)
target_link_libraries(cpn_format_test ${Boost_LIBRARIES} unfoldtacpn PetriParse Colored PQL)
target_link_libraries(string_query_test ${Boost_LIBRARIES} unfoldtacpn PetriParse Colored PQL)

add_test(NAME old_format_test COMMAND old_format_test)
add_test(NAME cpn_format_test COMMAND cpn_format_test)
add_test(NAME string_query_test COMMAND string_query_test)

set_tests_properties(old_format_test PROPERTIES
    ENVIRONMENT TEST_FILES=${CMAKE_CURRENT_SOURCE_DIR})

set_tests_properties(cpn_format_test PROPERTIES
    ENVIRONMENT TEST_FILES=${CMAKE_CURRENT_SOURCE_DIR})

set_tests_properties(string_query_test PROPERTIES
    ENVIRONMENT TEST_FILES=${CMAKE_CURRENT_SOURCE_DIR})