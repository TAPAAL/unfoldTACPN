project(unfoldtacpn C CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_subdirectory(PetriParse)
add_subdirectory(PetriEngine)

add_library(unfoldtacpn ${HEADER_FILES} unfoldtacpn.cpp)


add_executable(verifypn-${ARCH_TYPE} VerifyPN.cpp)
add_dependencies(unfoldtacpn glpk-ext)

target_link_libraries(unfoldtacpn PRIVATE PetriParse PetriEngine)
target_link_libraries(verifypn-${ARCH_TYPE} PRIVATE unfoldtacpn)

if (VERIFYPN_MC_Simplification)
    target_link_libraries(verifypn-${ARCH_TYPE} PUBLIC pthread)
endif(VERIFYPN_MC_Simplification)
install(TARGETS unfoldtacpn PQL Colored PetriEngine PetriParse verifypn-${ARCH_TYPE}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib/unfoldtacpn
    ARCHIVE DESTINATION lib/unfoldtacpn)

install(FILES ../include/unfoldtacpn.h DESTINATION include/)
