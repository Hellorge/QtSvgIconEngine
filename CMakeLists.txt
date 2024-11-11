cmake_minimum_required(VERSION 3.5)

project(SvgIconEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS SvgWidgets Concurrent)

set(SOURCES
    src/SvgIcon.cpp
    src/SvgIconEngine.cpp
    src/SvgIconButton.cpp
)

set(HEADERS
    include/SvgIcon.h
    include/SvgIconEngine.h
    include/SvgIconButton.h
)

add_library(SvgIconEngine STATIC ${SOURCES} ${HEADERS}
    src/SvgIconButton.cpp
    include/SvgIconButton.h)
target_link_libraries(SvgIconEngine PUBLIC Qt6::SvgWidgets Qt6::Concurrent)
target_include_directories(SvgIconEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
