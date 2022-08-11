# CMakeList.txt : CMake project for cpp_engine_lib, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

cmake_policy(SET CMP0091 NEW)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL") # Make all libraries and crap link to dynamic MSVC runtime (Bullet does this itself, which basically forces this project to as well)
project("cpp_engine" "CXX")

# Find Source Files

set(EC_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE EC_SRCS "${EC_SRC_DIR}/*.cpp")
file(GLOB_RECURSE EC_HDRS "${EC_SRC_DIR}/*.h")


add_library(cpp_engine_lib STATIC ${EC_SRCS})

if (EC_PROFILE)
	target_compile_definitions(cpp_engine_lib PUBLIC EC_PROFILE)
endif()



if (MSVC)
	if (EC_MAXWARN)
		target_compile_options(cpp_engine_lib PUBLIC /W4)
	endif()
	if(EC_BUILD_PARALLEL)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP5")
	endif()
endif()

target_include_directories(cpp_engine_lib PUBLIC ${EC_SRC_DIR})

# install library
install(TARGETS cpp_engine_lib
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION bin
		ARCHIVE DESTINATION lib)

set_target_properties(cpp_engine_lib
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO # Turning Extensions off for increased compatibility - see https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
		$<EC_PROFILE:EXE_LINKER_FLAGS ${CMAKE_LINKER_FLAGS} /Profile>
		) 

        
target_link_libraries(cpp_engine_lib INTERFACE ${ZLIB_LIBRARIES})

target_link_libraries(cpp_engine_lib INTERFACE ${SDL2_LIBRARIES})
target_include_directories(cpp_engine_lib PUBLIC ${SDL2_INCLUDE_DIRS})

target_link_libraries(cpp_engine_lib INTERFACE ${ASSIMP_LIBRARIES})
target_include_directories(cpp_engine_lib PUBLIC ${ASSIMP_INCLUDE_DIRS})

target_link_libraries(cpp_engine_lib INTERFACE ${SDL2_IMAGE_LIBRARIES})
target_include_directories(cpp_engine_lib PUBLIC ${SDL2_IMAGE_INCLUDE_DIRS})

target_link_libraries(cpp_engine_lib INTERFACE ${SDL_TTF_LIBRARIES})
target_include_directories(cpp_engine_lib PUBLIC ${SDL_TTF_INCLUDE_DIRS})

target_link_libraries(cpp_engine_lib INTERFACE ${BULLET_LIBRARIES})
target_include_directories(cpp_engine_lib PUBLIC ${BULLET_INCLUDE_DIRS})

target_link_libraries(cpp_engine_lib INTERFACE ${OPENGL_LIBRARIES})
target_include_directories(cpp_engine_lib PUBLIC ${OPENGL_INCLUDE_DIR})

target_link_libraries(cpp_engine_lib INTERFACE ${GLEW_LIBRARIES})
target_include_directories(cpp_engine_lib PUBLIC ${GLEW_INCLUDE_DIRS})

target_link_libraries(cpp_engine_lib INTERFACE ${OPENAL_LIBRARY})
target_include_directories(cpp_engine_lib PUBLIC ${OPENAL_INCLUDE_DIR})

target_link_libraries(cpp_engine_lib INTERFACE ${YAML_CPP_LIBRARIES})
target_include_directories(cpp_engine_lib PUBLIC ${YAMLCPP_INCLUDE_DIR})

if (DEFINED YAML_CPP_STATIC_DEFINE)
	target_compile_definitions(cpp_engine_lib PRIVATE YAML_CPP_STATIC_DEFINE=True)
endif()



# Enable Testing

include(CTest)

add_subdirectory(Game)
add_subdirectory(Tests)