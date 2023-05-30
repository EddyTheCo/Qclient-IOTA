cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
include(local_conf.cmake OPTIONAL)

include(FetchContent)
FetchContent_Declare(
	ccommon
	GIT_REPOSITORY https://github.com/EddyTheCo/Common.git
	GIT_TAG main
	)
FetchContent_MakeAvailable(ccommon)
version_from_git(
	LOG       OFF
	TIMESTAMP "%Y%m%d%H%M%S"
	)
project(QtIotaClient VERSION ${VERSION} DESCRIPTION "Qt-IOTA client library for REST API" LANGUAGES CXX)

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
	set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
		STRING "Choose the type of build." FORCE)
	# Set the possible values of build type for cmake-gui
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
		"Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Qt6 COMPONENTS Core Network)

if (Qt6_FOUND)
	qt_standard_project_setup()
endif(Qt6_FOUND)

add_library(qclient qclient.cpp src/qnode_response.cpp src/qnode_info.cpp src/qnode_tips.cpp src/qnode_blockId.cpp src/qnode_block.cpp src/qnode_outputs.cpp
	include/client/qnode_response.hpp include/client/qnode_info.hpp include/client/qnode_tips.hpp include/client/qclient.hpp include/client/qnode_blockId.hpp include/client/qnode_block.hpp
	include/client/qnode_outputs.hpp
	)


if (Qt6_FOUND)
	target_link_libraries(qclient PUBLIC  Qt6::Core Qt6::Network)
endif(Qt6_FOUND)

find_package(qblock 0 QUIET )
if(NOT qblock_FOUND AND NOT TARGET qblock)
	FetchContent_Declare(
		qblock
		GIT_REPOSITORY https://github.com/EddyTheCo/Qblock-IOTA.git
		GIT_TAG v0.0.1
		)
	FetchContent_MakeAvailable(qblock)
endif(NOT qblock_FOUND AND NOT TARGET qblock)
find_package(qblock 0 QUIET )
if(NOT qpow_FOUND AND NOT TARGET qpow)
	FetchContent_Declare(
		qpow
		GIT_REPOSITORY https://github.com/EddyTheCo/Qpow-IOTA.git
		GIT_TAG testing
		)
	FetchContent_MakeAvailable(qpow)
endif(NOT qpow_FOUND AND NOT TARGET qpow)

target_link_libraries(qclient PUBLIC qblock)
target_link_libraries(qclient PRIVATE qpow)


target_include_directories(qclient PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	"$<INSTALL_INTERFACE:include>")

install(TARGETS qclient EXPORT qclientTargets DESTINATION lib)
install(DIRECTORY include/ DESTINATION include/)

install(EXPORT qclientTargets
	FILE qclientTargets.cmake
	DESTINATION lib/cmake/qclient
	)
include(CMakePackageConfigHelpers)
# generate the config file that is includes the exports
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/qclientConfig.cmake"
	INSTALL_DESTINATION "lib/cmake/qclient"
	NO_SET_AND_CHECK_MACRO
	NO_CHECK_REQUIRED_COMPONENTS_MACRO
	)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/qclientConfigVersion.cmake"
	VERSION ${VERSION}
	COMPATIBILITY SameMajorVersion
	)
install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/qclientConfig.cmake
	${CMAKE_CURRENT_BINARY_DIR}/qclientConfigVersion.cmake
	DESTINATION lib/cmake/qclient
	)
export(EXPORT qclientTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/qclientTargets.cmake"
	)


if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	include(CTest)
	set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-v${SEMVER}-${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_CXX_COMPILER_ID}")
	if(NOT BUILD_SHARED_LIBS)
		set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-static")
	endif(NOT BUILD_SHARED_LIBS)
	if(USE_THREADS)
		set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-MThread")
	endif(USE_THREADS)
	include(CPack)
	add_subdirectory(examples)
endif()
#if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
#    add_subdirectory(tests)
#endif()
if(BUILD_DOCS)
	get_target_property(build_docs cmake_build_docs SOURCES)
	include(${build_docs})
endif()
