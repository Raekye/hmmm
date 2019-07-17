project(midori VERSION 0.0.1 DESCRIPTION "hmmm")

set(CMAKE_CXX_STANDARD 11)

set(SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/finite_automata.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/generator.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/lexer.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/regex.cpp
	)

add_library(midori SHARED ${SOURCES})
