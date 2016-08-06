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

find_package(PkgConfig REQUIRED)

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image)
pkg_search_module(SDL2_TTF REQUIRED SDL2_ttf)
pkg_search_module(LUA REQUIRED lua52)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
include_directories(${SDL2_TTF_INCLUDE_DIRS})
include_directories(${LUA_INCLUDE_DIRS})

add_executable(
	becoming_portland

	src/asset_manager.cpp
	src/dialog.cpp
	src/entities/npc.cpp
	src/entities/sprite.cpp
	src/map.cpp
	src/progress_bar.cpp
	src/screen_manager.cpp
	src/screens/main_screen.cpp
	src/screens/menu.cpp
	src/screens/opening.cpp
	src/screens/pause_menu.cpp
	src/state.cpp
	src/text.cpp
	src/tileset.cpp

	src/main.cpp
)
target_link_libraries(becoming_portland ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${LUA_LIBRARIES})
install(TARGETS becoming_portland RUNTIME DESTINATION ${BIN_DIR})

