cmake_minimum_required(VERSION 3.21)
project(ASTRA_Qt_Simple LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(ASTRA_GUI
    src/gui/main.cpp
    src/core/collision.c
    src/core/graph.c
    src/core/routing.c
)

target_include_directories(ASTRA_GUI PRIVATE src/core)
target_link_libraries(ASTRA_GUI PRIVATE Qt6::Widgets)

if(UNIX)
    target_link_libraries(ASTRA_GUI PRIVATE m)
endif()
