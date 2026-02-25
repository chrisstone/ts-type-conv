file(GLOB BLACKBOX_TESTS "*.ts")

foreach(TEST_FILE ${BLACKBOX_TESTS})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.toml")
        set(TEST_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.toml")
    else()
        set(TEST_CONFIG "")
    endif()

    if(${TEST_NAME} MATCHES "^proto_")
        set(OUT_EXT ".proto")
    else()
        set(OUT_EXT ".h")
    endif()

    # 1. Test parsing: run ts-type-conv and evaluate output.
    # Output file: ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}${OUT_EXT}
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}${OUT_EXT}
        COMMAND ${PROJECT_NAME} ${TEST_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}${OUT_EXT} ${TEST_CONFIG}
        DEPENDS ${PROJECT_NAME} ${TEST_FILE}
        COMMENT "Generating output for ${TEST_NAME}.ts via ts-type-conv..."
    )

    # 2. Add an explicit target for generation
    add_custom_target(generate_${TEST_NAME} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}${OUT_EXT})

    # 3. Test parsing and generation viability
    add_test(NAME ${TEST_NAME} COMMAND ${PROJECT_NAME} ${TEST_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}${OUT_EXT} ${TEST_CONFIG})
endforeach()
