FetchContent_Declare(
    qaddr_bundle
    GIT_REPOSITORY https://github.com/EddyTheCo/QAddrBundle.git
    GIT_TAG v0.2.2
    FIND_PACKAGE_ARGS 0.2 CONFIG
)
FetchContent_MakeAvailable(qaddr_bundle)
FetchContent_Declare(
    qslip10
    GIT_REPOSITORY https://github.com/EddyTheCo/Qslip10.git
    GIT_TAG v0.0.2
    FIND_PACKAGE_ARGS 0.0 CONFIG
)
FetchContent_MakeAvailable(qslip10)

FetchContent_Declare(
    qed25519
    GIT_REPOSITORY https://github.com/EddyTheCo/Qed25519.git
    GIT_TAG v0.0.2
    FIND_PACKAGE_ARGS 0.0 CONFIG
)
FetchContent_MakeAvailable(qed25519)

if (Qt6_FOUND)
    foreach(example "full_block_with_tagged_data_payload"
            "full_block_with_transaction_payload"
            "mint_nft"
            "alias_output"
            "foundry_output"
            "move_tokens_from_foundry"
            "alias_state_transition"
        )


    qt_add_executable( ${example}  ${example}.cpp)

    target_link_libraries(${example} PRIVATE qclient qaddr_bundle qslip10 qed25519)
    if (WIN32)
        add_custom_command(
            TARGET ${example} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${example}> $<TARGET_FILE_DIR:${example}>
            COMMAND_EXPAND_LISTS
        )
    endif (WIN32)


    endforeach()
endif(Qt6_FOUND)


