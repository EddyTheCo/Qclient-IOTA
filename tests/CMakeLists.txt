cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(testing VERSION 0.1 DESCRIPTION "diferent tests for the library" LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

qt_standard_project_setup()



qt_add_executable(full_block_with_tagged_data_payload
        full_block_with_tagged_data_payload.cpp
        )

if(EMSCRIPTEN)
	target_compile_options(full_block_with_tagged_data_payload PRIVATE -pthread )
	target_link_options(full_block_with_tagged_data_payload PRIVATE -pthread -sPTHREAD_POOL_SIZE=32 )
endif()
target_link_libraries(full_block_with_tagged_data_payload PRIVATE qclient)

include(FetchContent)
FetchContent_Declare(
        qaddr_bundle
        GIT_REPOSITORY git@github.com:EddyTheCo/QAddrBundle.git
        GIT_TAG main
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



qt_add_executable(full_block_with_transaction_payload
       full_block_with_transaction_payload.cpp
        )
add_test(NAME full_block_with_transaction_payload COMMAND full_block_with_transaction_payload https://api.testnet.shimmer.network ef4593558d0c3ed9e3f7a2de766d33093cd72372c800fa47ab5765c43ca006b5)
target_link_libraries(full_block_with_transaction_payload PRIVATE qclient qaddr_bundle qslip10 qed25519)


qt_add_executable(mint_nft
       mint_nft.cpp
        )
add_test(NAME mint_nft COMMAND mint_nft )
target_link_libraries(mint_nft PRIVATE qclient qaddr_bundle qslip10 qed25519)

