set(CMAKE_INCLUDE_CURRENT_DIR ON)

flex_target(pql_lexer "${CMAKE_CURRENT_SOURCE_DIR}/PQLQueryTokens.l"
                      "${CMAKE_CURRENT_SOURCE_DIR}/PQLQueryTokens.lexer.cpp")

bison_target(pql_parser "${CMAKE_CURRENT_SOURCE_DIR}/PQLQueryParser.y"
                        "${CMAKE_CURRENT_SOURCE_DIR}/PQLQueryParser.parser.cpp")

add_flex_bison_dependency(pql_lexer pql_parser)

add_library(PQL OBJECT ${BISON_pql_parser_OUTPUTS} ${FLEX_pql_lexer_OUTPUTS} Expressions.cpp PQL.cpp Contexts.cpp XMLPrinter.cpp Visitor.cpp SMCExpressions.cpp)

