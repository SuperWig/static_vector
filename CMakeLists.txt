cmake_minimum_required(VERSION 3.21)
project(static_vector LANGUAGES CXX)

option(DPM_BUILD_TESTS "Build the tests" ${PROJECT_IS_TOP_LEVEL})

add_library(static_vector INTERFACE)
add_library(dpm::static_vector ALIAS static_vector)

include(GNUInstallDirs)

target_include_directories(static_vector INTERFACE 
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_compile_features(static_vector INTERFACE cxx_std_20)

if (DPM_BUILD_TESTS)
	include(CTest)
	add_subdirectory(tests)
endif()

install(TARGETS static_vector EXPORT dpm-static_vector-config)
install(
	EXPORT dpm-static_vector-config
	NAMESPACE dpm::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dpm-static_vector
)
install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/
)