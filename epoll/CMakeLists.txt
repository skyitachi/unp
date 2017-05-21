cmake_minimum_required(VERSION 3.5)
project(epoll)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

file(GLOB SOURCE_FILES ${PROJECT_SOURCE_DIR}/*.c)

foreach (source ${SOURCE_FILES})
    string(REPLACE ".c" "" name ${source})
    string(REPLACE ${PROJECT_SOURCE_DIR}/ "" basename ${name})
    add_executable(${basename}.bin ${source})
endforeach (source ${SOURCE_FILES})
