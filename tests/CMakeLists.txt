include(FetchContent)

FetchContent_Declare(doctest
	GIT_REPOSITORY https://github.com/onqtam/doctest
	GIT_TAG 2.4.6
)

FetchContent_GetProperties(doctest)
if (NOT doctest_POPULATED)
	FetchContent_Populate(doctest)
	add_subdirectory(${doctest_SOURCE_DIR} ${doctest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(sv_test "test.cpp")
target_link_libraries(sv_test PRIVATE static_vector doctest_with_main)
add_test(NAME sv COMMAND sv_test)

add_executable(static "compile.cpp")
target_link_libraries(static PRIVATE static_vector)