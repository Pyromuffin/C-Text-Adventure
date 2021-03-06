cmake_minimum_required(VERSION 3.7)
project(C_Text_Adventure)

#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "/EHsc /std:c++latest /wd4996 /WX" )

include_directories("cereal/")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/" ${CMAKE_MODULE_PATH})

set(SFML_ROOT "${CMAKE_SOURCE_DIR}/SFML/" )
set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML COMPONENTS graphics window system REQUIRED)
include_directories(${SFML_INCLUDE_DIR})

include_directories("STB/")
include_directories("VK/Include/")
include_directories("glm/include/")

set(VULKAN_LIB_DIR "${CMAKE_SOURCE_DIR}/VK/Lib/")
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
set(VULKAN_LIBS "${VULKAN_LIB_DIR}vulkan-1.lib" "${VULKAN_LIB_DIR}VKstatic.1.lib" "${VULKAN_LIB_DIR}Debug/shaderc_combined.lib")
else()
set(VULKAN_LIBS "${VULKAN_LIB_DIR}vulkan-1.lib" "${VULKAN_LIB_DIR}VKstatic.1.lib" "${VULKAN_LIB_DIR}shaderc_combined.lib")
endif()


set(SOURCE_FILES
        main.cpp
        room.cpp
        commands.cpp items.h utility.h utility.cpp items.cpp IndexVector.h IndexVector.cpp state.cpp state.h StringHash.cpp StringHash.h CompileTimeStrings.h
		GameScript.cpp GameState.cpp TextProcessing.cpp parse.cpp Markup.cpp STB/stb_truetype.cpp Renderer.cpp ShaderCompiler.cpp GameRender.cpp
		
		)



add_executable(C_Text_Adventure ${SOURCE_FILES})
target_link_libraries(C_Text_Adventure ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} ${VULKAN_LIBS})
