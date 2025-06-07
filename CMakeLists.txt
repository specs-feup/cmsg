cmake_minimum_required(VERSION 3.20.0)

project(cmsg LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -Werror")

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)
set(EXAMPLES_DIR ${CMAKE_SOURCE_DIR}/examples)

add_library(cmsg STATIC
    ${SRC_DIR}/cmsg.c
)
set_target_properties(cmsg PROPERTIES OUTPUT_NAME "cmsg")
target_include_directories(cmsg PUBLIC ${INC_DIR})

install(DIRECTORY ${INC_DIR}/ DESTINATION include)
install(TARGETS cmsg
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

add_executable(cmsg_main ${SRC_DIR}/main.c)
target_include_directories(cmsg_main PRIVATE ${INC_DIR})
target_link_libraries(cmsg_main cmsg)

