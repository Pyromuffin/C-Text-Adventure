cmake_minimum_required(VERSION 3.7)
project(C_Text_Adventure)

#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "/EHsc /std:c++latest /wd4996")

include_directories("cereal/")

set(SOURCE_FILES
        main.cpp
        room.cpp
        room.h parse.h parse.cpp commands.h commands.cpp items.h utility.h utility.cpp items.cpp IndexVector.h IndexVector.cpp state.cpp state.h StringHash.cpp StringHash.h CompileTimeStrings.h
		GameScript.cpp GameScript.h GameState.h GameState.cpp
		
		)


#string(REGEX REPLACE "([^;]+)" "${CMAKE_CURRENT_SOURCE_DIR}/\\1.ixx" MODULE_FILE_PATHS "${MODULE_FILES}")
#string(REGEX REPLACE "([^;]+)" "${CMAKE_CURRENT_BINARY_DIR}/\\1.obj" OBJECT_FILE_PATHS "${MODULE_FILES}")

#add_custom_target(MODULES cl.exe /experimental:module /nologo -c ${MODULE_FILE_PATHS}
#WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#)

add_executable(C_Text_Adventure ${SOURCE_FILES})

#target_link_libraries(C_Text_Adventure ${OBJECT_FILE_PATHS})

#add_dependencies(C_Text_Adventure MODULES)