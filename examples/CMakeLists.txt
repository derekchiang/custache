SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_executable(main main.c)
add_dependencies(main custache)
target_link_libraries(main custache ${LINK_LIBRARIES})

add_executable(benchmark benchmark.c)
add_dependencies(benchmark custache)
target_link_libraries(benchmark custache ${LINK_LIBRARIES})
