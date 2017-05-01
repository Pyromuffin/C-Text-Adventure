cmake_minimum_required(VERSION 3.7)
project(C_Text_Adventure)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        main.c
        room.c
        room.h parse.h parse.c)

add_executable(C_Text_Adventure ${SOURCE_FILES})