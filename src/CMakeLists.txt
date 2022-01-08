project(unfoldtacpn C CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(PetriParse)
add_subdirectory(Colored)
add_subdirectory(PQL)

add_library(unfoldtacpn STATIC ${HEADER_FILES} unfoldtacpn.cpp)

target_link_libraries(unfoldtacpn PRIVATE PQL PetriParse Colored)

install(TARGETS unfoldtacpn PQL Colored PetriParse
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib/unfoldtacpn
    ARCHIVE DESTINATION lib/unfoldtacpn)

install(FILES ../include/unfoldtacpn.h ../include/TAPNBuilderInterface.h DESTINATION include/)
install(FILES ../include/PQL/PQL.h ../include/PQL/Visitor.h ../include/PQL/Expressions.h  DESTINATION include/PQL/)
install(FILES ../include/Colored/ColoredNetStructures.h
              ../include/Colored/ColoredPetriNetBuilder.h
              ../include/Colored/Colors.h
              ../include/Colored/Multiset.h
              ../include/Colored/Expressions.h
              ../include/Colored/TimeInterval.h
              ../include/Colored/TimeInvariant.h  DESTINATION include/Colored/)