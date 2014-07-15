cmake_minimum_required(VERSION 2.8)
project(custache)

SET(CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH})

SET(BLOCKS_CC_FLAGS "-fblocks")
SET(BLOCKS_LINKER_FLAGS "-lBlocksRuntime")

SET(CMAKE_BUILD_TYPE Debug)

add_definitions(${BLOCKS_CC_FLAGS})
add_definitions(-DE4C_THREADSAFE)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${BLOCKS_LINKER_FLAGS}")

find_package(PkgConfig)
pkg_check_modules(APR apr-1)

link_directories(${LINK_DIRECTORIES} ${APR_LIBRARY_DIRS})
include_directories(${INCLUDE_DIRECTORIES} ${APR_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)
link_libraries(${LINK_LIBRARIES} ${APR_STATIC_LIBRARIES})

file(GLOB HEADERS "include/*.h")
install(FILES ${HEADERS} DESTINATION include)

add_subdirectory(src)
add_subdirectory(examples)
