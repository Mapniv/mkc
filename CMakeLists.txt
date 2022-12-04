cmake_minimum_required (VERSION 3.10)

project (mkc VERSION 0.0.0)


file (GLOB_RECURSE SOURCE_FILES "src/*.c")

add_executable (mkc ${SOURCE_FILES})
target_include_directories (
	mkc PUBLIC
	${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}
)

enable_testing ()
