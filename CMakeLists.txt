cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
include(local_conf.cmake OPTIONAL)
project(QtIotaClient VERSION 0.2 DESCRIPTION "IOTA-QT client library" LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
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
set(CMAKE_CXX_EXTENSIONS OFF)


add_library(qclient qclient.cpp src/qnode_response.cpp src/qnode_info.cpp src/qnode_tips.cpp src/qnode_blockId.cpp src/qnode_block.cpp src/qnode_outputs.cpp
    include/client/qnode_response.hpp include/client/qnode_info.hpp include/client/qnode_tips.hpp include/client/qclient.hpp include/client/qnode_blockId.hpp include/client/qnode_block.hpp
    include/client/qnode_outputs.hpp
)
include(FetchContent)
FetchContent_Declare(
        qblock
        GIT_REPOSITORY git@github.com:EddyTheCo/Qblock-IOTA.git
        GIT_TAG version-0.2
        )
FetchContent_MakeAvailable(qblock)
FetchContent_Declare(
        qpow
        GIT_REPOSITORY git@github.com:EddyTheCo/Qpow-IOTA.git
        GIT_TAG main
        )
FetchContent_MakeAvailable(qpow)
target_link_libraries(qclient PUBLIC qblock)
target_link_libraries(qclient PRIVATE qpow)

find_package(Qt6 REQUIRED COMPONENTS Core Network)
target_link_libraries(qclient PUBLIC  Qt6::Core Qt6::Network)

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
	VERSION "0.1.1.0"
	COMPATIBILITY AnyNewerVersion
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
endif()
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
endif()
