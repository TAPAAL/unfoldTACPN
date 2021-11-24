project(unfoldtacpn C CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_subdirectory(PetriParse)
add_subdirectory(Colored)
add_subdirectory(PQL)

add_library(unfoldtacpn ${HEADER_FILES} unfoldtacpn.cpp)

target_link_libraries(unfoldtacpn PRIVATE PetriParse PetriEngine)

install(TARGETS unfoldtacpn PQL Colored PetriParse
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib/unfoldtacpn
    ARCHIVE DESTINATION lib/unfoldtacpn)

install(FILES ../include/unfoldtacpn.h DESTINATION include/)
