cmake_minimum_required(VERSION 3.6)
project(tcpcliserv)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -lunp")

file(GLOB SOURCE_FILES ${PROJECT_SOURCE_DIR}/*.c)

foreach (source ${SOURCE_FILES})
    string(REPLACE ".c" "" name ${source})
    string(REPLACE ${PROJECT_SOURCE_DIR}/ "" basename ${name})
    add_executable(${basename} ${source})
endforeach (source ${SOURCE_FILES})

