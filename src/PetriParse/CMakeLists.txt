set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(PetriParse OBJECT ${HEADER_FILES} PNMLParser.cpp QueryXMLParser.cpp)
add_dependencies(PetriParse rapidxml-ext)

