cmake_minimum_required(VERSION 3.12.4)
set(LRCALC_MAJOR_VERSION 2)
set(LRCALC_MINOR_VERSION 1)
set(LRCALC_PATCH_VERSION 0)
set(LRCALC_VERSION ${LRCALC_MAJOR_VERSION}.${LRCALC_MINOR_VERSION}.${LRCALC_PATCH_VERSION})
project(lrcalc LANGUAGES C CXX VERSION ${LRCALC_VERSION})

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

if(MSVC)
	if(CMAKE_BUILD_TYPE)
		message("CMAKE_BUILD_TYPE is set to `${CMAKE_BUILD_TYPE}`, but would be ignored.")
	endif()
else()
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE "Release")
		message("CMAKE_BUILD_TYPE is not specified: Use Release by default.")
	endif()
	if(NOT((CMAKE_BUILD_TYPE STREQUAL "Debug") OR (CMAKE_BUILD_TYPE STREQUAL "Release")))
		message(FATAL_ERROR "CMAKE_BUILD_TYPE must be 'Debug' or 'Release' (case-sensitive)")
	endif()
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(cmake/utils.cmake)  # for fix_default_cxx_flags, set_flags, set_default_property

fix_default_cxx_flags()
set_flags()

set(MY_TARGET lrcalc)

add_library(${MY_TARGET} STATIC
	src/optshape.cpp
	src/schur.cpp
	src/vectarg.cpp
	include/lrcalc/ivector.hpp
	include/lrcalc/ivlincomb.hpp
	include/lrcalc/ivlist.hpp
	include/lrcalc/lrcoef.hpp
	include/lrcalc/lriter.hpp
	include/lrcalc/maple.hpp
	include/lrcalc/optshape.hpp
	include/lrcalc/part.hpp
	include/lrcalc/perm.hpp
	include/lrcalc/cpp_lib.hpp
	include/lrcalc/schublib.hpp
	include/lrcalc/schur.hpp
	include/lrcalc/vectarg.hpp
)

find_package(Threads REQUIRED)
link_directories(${CMAKE_SOURCE_DIR})
target_link_libraries(${MY_TARGET} INTERFACE lrcalc_helper "-pthread" dl)
if(NOT MSVC)
	target_link_libraries(${MY_TARGET} INTERFACE stdc++fs)
	if(NOT CMAKE_COMPILER_IS_GNUCXX)
		target_link_libraries(${MY_TARGET} INTERFACE c++abi)
	endif()
endif()

target_include_directories(${MY_TARGET} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	INTERFACE
	$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)

set_default_property(${MY_TARGET} .)

include(CMakePackageConfigHelpers)
install(DIRECTORY "include/" DESTINATION include)
set(cmake_package_name LRCalc)
set(targets_export_name ${cmake_package_name}Targets CACHE INTERNAL "")
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated" CACHE INTERNAL "")
set(cmake_files_install_dir "lib/cmake/${cmake_package_name}")
set(version_file "${generated_dir}/${cmake_package_name}ConfigVersion.cmake")
write_basic_package_version_file(${version_file} VERSION ${LRCALC_VERSION} COMPATIBILITY AnyNewerVersion)
install(EXPORT ${targets_export_name}
	NAMESPACE ${cmake_package_name}::
	DESTINATION ${cmake_files_install_dir}
)
set(config_file "${generated_dir}/${cmake_package_name}Config.cmake")
configure_package_config_file("cmake/Config.cmake.in"
	"${config_file}" INSTALL_DESTINATION ${cmake_files_install_dir})
install(FILES ${version_file} ${config_file}
	DESTINATION ${cmake_files_install_dir})
install(TARGETS ${MY_TARGET}
	EXPORT ${targets_export_name}
	CONFIGURATIONS Debug Release
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib)

set(MY_TARGET lrcalc-bin)
add_executable(${MY_TARGET} src/lrcalc.cpp)
set_default_property(${MY_TARGET} .)
target_link_libraries(${MY_TARGET} PRIVATE lrcalc)
set_target_properties(${MY_TARGET} PROPERTIES OUTPUT_NAME lrcalc)

set(MY_TARGET schubmult)
add_executable(${MY_TARGET} src/schubmult.cpp)
set_default_property(${MY_TARGET} .)
target_link_libraries(${MY_TARGET} PRIVATE lrcalc)

set(MY_TARGET allparts)
add_executable(${MY_TARGET} src/allparts.cpp)
set_default_property(${MY_TARGET} .)
target_link_libraries(${MY_TARGET} PRIVATE lrcalc)

set(MY_TARGET allperms)
add_executable(${MY_TARGET} src/allperms.cpp)
set_default_property(${MY_TARGET} .)
target_link_libraries(${MY_TARGET} PRIVATE lrcalc)

set(MY_TARGET allstrings)
add_executable(${MY_TARGET} src/allstrings.cpp)
set_default_property(${MY_TARGET} .)
target_link_libraries(${MY_TARGET} PRIVATE lrcalc)

set(MY_TARGET test_fusion)
add_executable(${MY_TARGET} src/test_fusion.cpp)
set_default_property(${MY_TARGET} .)
target_link_libraries(${MY_TARGET} PRIVATE lrcalc)

set(MY_TARGET test_lrcoef)
add_executable(${MY_TARGET} src/test_lrcoef.cpp)
set_default_property(${MY_TARGET} .)
target_link_libraries(${MY_TARGET} PRIVATE lrcalc)

set(MY_TARGET test_lrmult)
add_executable(${MY_TARGET} src/test_lrmult.cpp)
set_default_property(${MY_TARGET} .)
target_link_libraries(${MY_TARGET} PRIVATE lrcalc)

set(MY_TARGET test_lrskew)
add_executable(${MY_TARGET} src/test_lrskew.cpp)
set_default_property(${MY_TARGET} .)
target_link_libraries(${MY_TARGET} PRIVATE lrcalc)

set(MY_TARGET test_partiter)
add_executable(${MY_TARGET} src/test_partiter.cpp)
set_default_property(${MY_TARGET} .)
target_link_libraries(${MY_TARGET} PRIVATE lrcalc)

set(MY_TARGET test_schubmult)
add_executable(${MY_TARGET} src/test_schubmult.cpp)
set_default_property(${MY_TARGET} .)
target_link_libraries(${MY_TARGET} PRIVATE lrcalc)
