set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_subdirectory(PetriParse)
add_subdirectory(PetriEngine)

add_library(unfoldtacpn ${HEADER_FILES}
    unfoldtacpn.cpp PetriEngine/Colored/ColoredPetriNetBuilder.cpp PetriEngine/Colored/Colors.cpp PetriEngine/Colored/Multiset.cpp 
    PetriEngine/Colored/TimeInterval.cpp PetriEngine/Colored/TimeInvariant.cpp 
    PetriEngine/PQL/Contexts.cpp PetriEngine/PQL/Expressions.cpp PetriEngine/PQL/PQL.cpp PetriEngine/PetriNet.cpp 
    PetriEngine/PetriNetBuilder.cpp PetriEngine/Reducer.cpp PetriParse/PNMLParser.cpp PetriParse/QueryBinaryParser.cpp
    PetriParse/QueryXMLParser.cpp)
add_executable(verifypn-${ARCH_TYPE} VerifyPN.cpp)
add_dependencies(unfoldtacpn glpk-ext)

target_link_libraries(verifypn-${ARCH_TYPE} PRIVATE PetriEngine PetriParse unfoldtacpn)

if (VERIFYPN_MC_Simplification)
    target_link_libraries(verifypn-${ARCH_TYPE} PUBLIC pthread)
endif(VERIFYPN_MC_Simplification)
install(TARGETS unfoldtacpn verifypn-${ARCH_TYPE}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ARCHIVE DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

install(FILES unfoldtacpn.h DESTINATION include/)
