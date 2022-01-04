set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(Colored STATIC ColoredPetriNetBuilder.cpp
    Colors.cpp
    Multiset.cpp
    TimeInterval.cpp
    TimeInvariant.cpp
    Expression.cpp)
add_dependencies(Colored rapidxml-ext)
