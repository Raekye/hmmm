project(midori VERSION 0.0.1 DESCRIPTION "hmmm")

set(SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/finite_automata.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/regex_ast.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/lexer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/regex_engine.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/generator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/lang.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/types.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/type_checker.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/codegen.cpp
	)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs core interpreter mcjit orcjit native)

add_library(midori SHARED ${SOURCES})
target_compile_options(midori PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-unknown-pragmas)
target_link_libraries(midori PUBLIC -rdynamic)
target_link_libraries(midori PUBLIC ${llvm_libs})
#target_link_libraries(midori PUBLIC coverage)
