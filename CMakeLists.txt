cmake_minimum_required(VERSION 3.24)
include(${CMAKE_CURRENT_BINARY_DIR}/local_conf.cmake OPTIONAL)

include(FetchContent)
FetchContent_Declare(
	ccommon
	GIT_REPOSITORY https://github.com/EddyTheCo/Common.git
	GIT_TAG use_IFW
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
	GIT_TAG use_IFW
	FIND_PACKAGE_ARGS 0.3 CONFIG
	)
FetchContent_MakeAvailable(QtIotaBlock)


FetchContent_Declare(
	QtIotaPow
	GIT_REPOSITORY https://github.com/EddyTheCo/Qpow-IOTA.git
	GIT_TAG use_IFW
	FIND_PACKAGE_ARGS 0.2 CONFIG
	)
FetchContent_MakeAvailable(QtIotaPow)

find_package(Qt6 COMPONENTS Core Network)

if (Qt6_FOUND)
	qt_standard_project_setup()


	add_library(qclient qclient.cpp src/qnode_response.cpp src/qnode_info.cpp src/qnode_tips.cpp src/qnode_blockId.cpp src/qnode_block.cpp src/qnode_outputs.cpp
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
		)
	install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/"
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Esterv
		COMPONENT client
		)

	install(EXPORT ${PROJECT_NAME}-config
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
		NAMESPACE ${PROJECT_NAME}::
		COMPONENT ${PROJECT_NAME}
		)
	include(CMakePackageConfigHelpers)
	configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
		"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
		INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
		)
	write_basic_package_version_file(
		"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
		VERSION ${VERSION}
		COMPATIBILITY SameMajorVersion
		)
	install(FILES
		${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
		${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
		COMPONENT client
		)
	export(EXPORT ${PROJECT_NAME}-config
		FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
		)
	if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
		set(cpack_file_name "${PROJECT_NAME}-v${SEMVER}-${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_CXX_COMPILER_ID}")
		if(NOT BUILD_SHARED_LIBS)
			set(cpack_file_name "${cpack_file_name}-static")
		endif(NOT BUILD_SHARED_LIBS)
		if(USE_THREADS)
			set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-MThread")
		endif(USE_THREADS)
		set(CPACK_PACKAGE_FILE_NAME ${cpack_file_name})
		include(CPack)
		include(CPackIFW)
		cpack_add_component(client)
		cpack_ifw_configure_component(client LICENSES "License" ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
		cpack_ifw_add_repository("Repo" URL "${REPO_URL}/packages/${CMAKE_SYSTEM_NAME}/" )
	endif()
	#if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
	#	include(CTest)
	#    add_subdirectory(tests)
	#endif()
endif(Qt6_FOUND)
if(BUILD_DOCS)
	build_docs()
endif()
