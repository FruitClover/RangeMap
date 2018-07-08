cmake_minimum_required(VERSION 3.0)
project(RangeMap)

option(RANGEMAP_ENABLE_ASAN "Enable ASan." ON)
option(RANGEMAP_ENABLE_UBSAN "Enable UBsan." ON)

set(CMAKE_CXX_FLAGS "-std=c++17 -W -Wall -Wextra")
#
# Implement cusom options
#
if (RANGEMAP_ENABLE_ASAN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-common")
  set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-common")
endif()

if (RANGEMAP_ENABLE_UBSAN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
  set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -fsanitize=underfined")
endif()

# Update git submodules
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.gitmodules")
  execute_process(COMMAND git submodule update --init --recursive
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

add_subdirectory(rangemap)

enable_testing()
add_subdirectory(third_party/googletest)
add_subdirectory(tests)
