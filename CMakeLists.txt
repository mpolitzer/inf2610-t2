cmake_minimum_required(VERSION 3.2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -O2 -Wall -pedantic -std=c11 -g -I../include")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")

include(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(GL   REQUIRED gl)
add_executable(defered main.c tz/tz/gfx/gl_core_3_3.c tz/tz/gfx/rply-1.1.4/rply.c)
target_link_libraries(defered SDL2 GL m dl)

