cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(testing VERSION 0.1 DESCRIPTION "diferent tests for the library" LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
qt_standard_project_setup()
include(FetchContent)
FetchContent_Declare(
        qpow
        GIT_REPOSITORY git@github.com:EddyTheCo/Qpow-IOTA.git
        GIT_TAG main
        )
FetchContent_MakeAvailable(qpow)

FetchContent_Declare(
        qblock
        GIT_REPOSITORY git@github.com:EddyTheCo/Qblock-IOTA.git
        GIT_TAG main
        )
FetchContent_MakeAvailable(qblock)

qt_add_executable(full_block_with_tagged_data_payload
        full_block_with_tagged_data_payload.cpp
        )

add_test(NAME full_block_with_tagged_data_payload COMMAND full_block_with_tagged_data_payload)
target_link_libraries(full_block_with_tagged_data_payload PRIVATE qclient qpow qblock)


FetchContent_Declare(
        qslip10
        GIT_REPOSITORY git@github.com:EddyTheCo/Qslip10.git
        GIT_TAG main
        )
FetchContent_MakeAvailable(qslip10)


FetchContent_Declare(
        qbech32
        GIT_REPOSITORY git@github.com:EddyTheCo/Qbech32.git
        GIT_TAG main
        )
FetchContent_MakeAvailable(qbech32)
FetchContent_Declare(
        qed25519
        GIT_REPOSITORY git@github.com:EddyTheCo/Qed25519.git
        GIT_TAG main
        )
FetchContent_MakeAvailable(qed25519)
option(USE_SODIUM "On or off" ON)
qt_add_executable(full_block_with_transaction_payload
        full_block_with_transaction_payload.cpp
        )
add_test(NAME full_block_with_transaction_payload COMMAND full_block_with_transaction_payload)
target_link_libraries(full_block_with_transaction_payload PRIVATE qclient qpow qblock qed25519 qslip10 qbech32)
