cmake_minimum_required(VERSION 3.0)
project(portland)

cmake_policy(SET CMP0054 NEW)

# Use our modified FindSDL2* modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${portland_SOURCE_DIR}/cmake")

# Bump up warning levels appropriately for clang, gcc & msvc and build in debug mode
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-deprecated-declarations -std=c++14 -DCHAISCRIPT_NO_THREADS")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
endif()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  find_package(SFML COMPONENTS audio graphics main system window)
else()
  find_package(SFML COMPONENTS audio graphics system window)
endif()

include_directories(
  ${SFML_INCLUDE_DIR}
  "vendor/ChaiScript/include"
  "vendor/json"
)

set(PLATFORM_LIBRARIES dl)

file(GLOB_RECURSE portland_sources src/*.cpp)
add_executable(portland ${portland_sources})
target_link_libraries(
  portland
  ${SFML_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)
