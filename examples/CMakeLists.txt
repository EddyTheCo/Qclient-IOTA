include(FetchContent)
find_package(qaddr_bundle 0 QUIET )
if(NOT qaddr_bundle_FOUND AND NOT TARGET qaddr_bundle)
	FetchContent_Declare(
		qaddr_bundle
		GIT_REPOSITORY https://github.com/EddyTheCo/QAddrBundle.git
		GIT_TAG v0.0.1
		)
	FetchContent_MakeAvailable(qaddr_bundle)
endif(NOT qaddr_bundle_FOUND AND NOT TARGET qaddr_bundle)

find_package(qslip10 0 QUIET )
if(NOT qslip10_FOUND AND NOT TARGET qslip10)
	FetchContent_Declare(
		qslip10
		GIT_REPOSITORY https://github.com/EddyTheCo/Qslip10.git
		GIT_TAG v0.0.1
		)
	FetchContent_MakeAvailable(qslip10)
endif(NOT qslip10_FOUND AND NOT TARGET qslip10)

find_package(qed25519 0 QUIET )
if(NOT qed25519_FOUND AND NOT TARGET qed25519)
	FetchContent_Declare(
		qed25519
		GIT_REPOSITORY https://github.com/EddyTheCo/Qed25519.git
		GIT_TAG v0.0.1
		)
	FetchContent_MakeAvailable(qed25519)
endif(NOT qed25519_FOUND AND NOT TARGET qed25519)


foreach(example "full_block_with_tagged_data_payload"
		"full_block_with_transaction_payload"
		"mint_nft"
		"alias_output"
		"foundry_output"
		"move_tokens_from_foundry"
		"alias_state_transition"
		)

	if (Qt6_FOUND)
		qt_add_executable( ${example}  ${example}.cpp)

		target_link_libraries(${example} PUBLIC qclient qblock qaddr_bundle qslip10 qed25519)
		if (WIN32 AND BUILD_SHARED_LIBS)
			add_custom_command(
				TARGET ${example} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${example}> $<TARGET_FILE_DIR:${example}>
				COMMAND_EXPAND_LISTS
				)
		endif ()
	endif(Qt6_FOUND)

endforeach()
