cmake_minimum_required(VERSION 3.0)
project(becoming_portland)

# Use our modified FindSDL2* modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${becoming_portland_SOURCE_DIR}/cmake")
set(BIN_DIR ${becoming_portland_SOURCE_DIR}/bin)

# Bump up warning levels appropriately for clang, gcc & msvc and build in debug mode
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
endif()

#find_package(PkgConfig REQUIRED)

find_package(SDL2 REQUIRED sdl2)
find_package(SDL2_IMAGE REQUIRED SDL2_image)
find_package(SDL2_TTF REQUIRED SDL2_ttf)
find_package(LUA REQUIRED lua52)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
include_directories(${SDL2_TTF_INCLUDE_DIRS})
include_directories(${LUA_INCLUDE_DIRS})
include_directories("vendor/spdlog/include")
include_directories("vendor/selene/include")
include_directories("vendor/json")

file(GLOB_RECURSE portland_sources src/*.cpp)
add_executable(
	becoming_portland

  ${portland_sources}
)
target_link_libraries(becoming_portland ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${LUA_LIBRARIES} pthread)
install(TARGETS becoming_portland RUNTIME DESTINATION ${BIN_DIR})

