cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
include(${CMAKE_CURRENT_BINARY_DIR}/local_conf.cmake OPTIONAL)

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
project(IotaClient VERSION ${VERSION} DESCRIPTION "Methods to comunicate with the REST API of Iota nodes" LANGUAGES CXX)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
include(GNUInstallDirs)

FetchContent_Declare(
    qblock
    GIT_REPOSITORY https://github.com/EddyTheCo/Qblock-IOTA.git
    GIT_TAG develop
    FIND_PACKAGE_ARGS 0.3 CONFIG
)
FetchContent_MakeAvailable(qblock)


FetchContent_Declare(
    qpow
    GIT_REPOSITORY https://github.com/EddyTheCo/Qpow-IOTA.git
    GIT_TAG v0.2.1
    FIND_PACKAGE_ARGS 0.2 CONFIG
)
FetchContent_MakeAvailable(qpow)

find_package(Qt6 COMPONENTS Core Network)

if (Qt6_FOUND)
    qt_standard_project_setup()
endif(Qt6_FOUND)

add_library(client qclient.cpp src/qnode_response.cpp src/qnode_info.cpp src/qnode_tips.cpp src/qnode_blockId.cpp src/qnode_block.cpp src/qnode_outputs.cpp
    include/client/qnode_response.hpp include/client/qnode_info.hpp include/client/qnode_tips.hpp include/client/qclient.hpp include/client/qnode_blockId.hpp include/client/qnode_block.hpp
    include/client/qnode_outputs.hpp)

add_library(${PROJECT_NAME}::client ALIAS client)
target_compile_definitions(client PRIVATE WINDOWS_QCLIENT)
if (Qt6_FOUND)
    target_link_libraries(client PUBLIC  Qt6::Core Qt6::Network)
endif(Qt6_FOUND)

target_link_libraries(client PUBLIC qblock )
target_link_libraries(client PRIVATE qpow)

target_include_directories(client PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    "$<INSTALL_INTERFACE:/Esterv/${PROJECT_NAME}>")

install(TARGETS client
    EXPORT ${PROJECT_NAME}-config
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT client
)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Esterv/${PROJECT_NAME}
    COMPONENT client
)

install(EXPORT ${PROJECT_NAME}-config
    FILE ${PROJECT_NAME}-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Esterv
    NAMESPACE ${PROJECT_NAME}::
    COMPONENT ${PROJECT_NAME}
)
include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Esterv
        )
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${VERSION}
        COMPATIBILITY SameMajorVersion
        )
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Esterv
        )
export(EXPORT ${PROJECT_NAME}-config
        FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        )
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CPACK_PACKAGE_CONTACT "estervtech")
    include(CTest)
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-v${SEMVER}-${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_CXX_COMPILER_ID}")
    if(NOT BUILD_SHARED_LIBS)
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-static")
    endif(NOT BUILD_SHARED_LIBS)
    if(USE_THREADS)
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-MThread")
    endif(USE_THREADS)
    include(CPack)
endif()
#if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
#    add_subdirectory(tests)
#endif()
if(BUILD_DOCS)
    get_target_property(build_docs cmake_build_docs SOURCES)
    include(${build_docs})
endif()
