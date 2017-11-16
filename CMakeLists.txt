cmake_minimum_required(VERSION 3.7)
project(C_Text_Adventure)

#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "/EHsc /std:c++latest /wd4996")

set(SDL2_DIR "SDL")
set(SDL2_TTF_DIR "SDL_TTF") 

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(SDL2_TTF REQUIRED)
include_directories(${SDL2_TTF_INCLUDE_DIRS})

include_directories("cereal/")


set(SOURCE_FILES
        main.cpp
        room.cpp
        commands.cpp items.h utility.h utility.cpp items.cpp IndexVector.h IndexVector.cpp state.cpp state.h StringHash.cpp StringHash.h CompileTimeStrings.h
		GameScript.cpp GameState.cpp TextProcessing.cpp parse.cpp
		
		)



add_executable(C_Text_Adventure ${SOURCE_FILES})
target_link_libraries(C_Text_Adventure ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})
