project(unfoldtacpn C CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_subdirectory(PetriParse)
add_subdirectory(PetriEngine)

add_library(unfoldtacpn ${HEADER_FILES} unfoldtacpn.cpp)

add_dependencies(unfoldtacpn glpk-ext)
target_link_libraries(unfoldtacpn PRIVATE PetriParse PetriEngine)

install(TARGETS unfoldtacpn PQL Colored PetriEngine PetriParse
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib/unfoldtacpn
    ARCHIVE DESTINATION lib/unfoldtacpn)

install(FILES ../include/unfoldtacpn.h DESTINATION include/)
