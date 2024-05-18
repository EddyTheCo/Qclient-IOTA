cmake_minimum_required(VERSION 3.24)
include(${CMAKE_CURRENT_BINARY_DIR}/local_conf.cmake OPTIONAL)

include(FetchContent)
FetchContent_Declare(
	ccommon
	GIT_REPOSITORY https://github.com/EddyTheCo/Common.git
	GIT_TAG v0.4.0
	)
FetchContent_MakeAvailable(ccommon)
version_from_git(
	LOG       OFF
	TIMESTAMP "%Y%m%d%H%M%S"
	)

project(IotaClient VERSION ${VERSION} DESCRIPTION "Methods to comunicate with the REST API of Iota nodes" LANGUAGES CXX)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
include(GNUInstallDirs)

FetchContent_Declare(
	QtIotaBlock
	GIT_REPOSITORY https://github.com/EddyTheCo/Qblock-IOTA.git
	GIT_TAG v0.4.1
	FIND_PACKAGE_ARGS 0.4 CONFIG
	)
FetchContent_MakeAvailable(QtIotaBlock)


FetchContent_Declare(
	QtIotaPow
	GIT_REPOSITORY https://github.com/EddyTheCo/Qpow-IOTA.git
	GIT_TAG v0.3.1
	FIND_PACKAGE_ARGS 0.3 CONFIG
	)
FetchContent_MakeAvailable(QtIotaPow)

find_package(Qt6 COMPONENTS Core Network)

if (Qt6_FOUND)
	qt_standard_project_setup()
	add_library(qclient src/qclient.cpp src/qnode_response.cpp src/qnode_info.cpp src/qnode_tips.cpp src/qnode_blockId.cpp src/qnode_block.cpp src/qnode_outputs.cpp
		include/client/qnode_response.hpp include/client/qnode_info.hpp include/client/qnode_tips.hpp include/client/qclient.hpp include/client/qnode_blockId.hpp include/client/qnode_block.hpp
		include/client/qnode_outputs.hpp)

	add_library(${PROJECT_NAME}::qclient ALIAS qclient)
	set_target_properties(qclient PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_MAJOR})

	target_compile_definitions(qclient PRIVATE WINDOWS_QCLIENT)

	target_link_libraries(qclient PUBLIC  Qt6::Core Qt6::Network)


	target_include_directories(qclient PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Esterv/>")

	target_link_libraries(qclient PUBLIC QtIotaBlock::qblock )
	target_link_libraries(qclient PRIVATE QtIotaPow::qpow)


	install(TARGETS qclient
		EXPORT ${PROJECT_NAME}-config
		DESTINATION ${CMAKE_INSTALL_LIBDIR}
		COMPONENT client
		RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT client
		)
	install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/"
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Esterv
		COMPONENT ${PROJECT_NAME}
		)

	build_exports()
	if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
		build_cpack()
	endif(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
endif(Qt6_FOUND)
if(BUILD_DOCS)
	build_docs()
endif(BUILD_DOCS)
