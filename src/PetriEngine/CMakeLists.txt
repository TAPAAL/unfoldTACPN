set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(Colored)
add_subdirectory(PQL)

add_library(PetriEngine ${HEADER_FILES} 
PetriNet.cpp
PetriNetBuilder.cpp
Reducer.cpp
)

target_link_libraries(PetriEngine PRIVATE Colored PQL)
