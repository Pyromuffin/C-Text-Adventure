cmake_minimum_required(VERSION 3.7)
project(C_Text_Adventure)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-g2 -gdwarf-2 /Zi /potato")

set(SOURCE_FILES
        main.cpp
        room.cpp
        room.h parse.h parse.cpp commands.h commands.cpp items.h utility.h utility.cpp items.cpp IndexVector.h IndexVector.cpp state.cpp state.h StringHash.c StringHash.h)

add_executable(C_Text_Adventure ${SOURCE_FILES})