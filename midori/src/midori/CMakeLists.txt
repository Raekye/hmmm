project(midori VERSION 0.0.1 DESCRIPTION "hmmm")

set(SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/finite_automata.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/regex_ast.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/lexer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/regex_engine.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/generator.cpp
	)

add_library(midori SHARED ${SOURCES})
