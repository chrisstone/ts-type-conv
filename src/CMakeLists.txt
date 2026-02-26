add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    lexer.cpp
    main.cpp
    parser.cpp
    config.cpp
    emit/codegen_cpp.cpp
    emit/codegen_proto.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/contrib/tomlplusplus)
