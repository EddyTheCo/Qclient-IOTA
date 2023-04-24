cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(examples VERSION 0.1 DESCRIPTION "diferent examples for the library" LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

qt_standard_project_setup()

include(FetchContent)
FetchContent_Declare(
    qaddr_bundle
    GIT_REPOSITORY git@github.com:EddyTheCo/QAddrBundle.git
    GIT_TAG develop
)
FetchContent_MakeAvailable(qaddr_bundle)
FetchContent_Declare(
    qslip10
    GIT_REPOSITORY git@github.com:EddyTheCo/Qslip10.git
    GIT_TAG main
)
FetchContent_MakeAvailable(qslip10)
FetchContent_Declare(
    qed25519
    GIT_REPOSITORY git@github.com:EddyTheCo/Qed25519.git
    GIT_TAG main
)
FetchContent_MakeAvailable(qed25519)


foreach(example "full_block_with_tagged_data_payload"
        "full_block_with_transaction_payload"
        "mint_nft"
        "alias_output"
        "foundry_output"
        "move_tokens_from_foundry"
        "alias_state_transition"
    )

message(STATUS "Setting example   ${example}")
qt_add_executable( ${example}  ${example}.cpp)


target_link_libraries(${example} PRIVATE qclient qaddr_bundle qslip10 qed25519)

endforeach()
