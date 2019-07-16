project(hmmm VERSION 0.0.1 DESCRIPTION "hmmm")

add_subdirectory(midori)

set(CMAKE_CXX_STANDARD 11)

add_executable(hmmm
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	)
target_link_libraries(hmmm midori)
