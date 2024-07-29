cmake_minimum_required(VERSION 3.5)

project(SvgIconEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS SvgWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS SvgWidgets Concurrent)

set(SOURCES
    src/SvgIcon.cpp
    src/SvgIconEngine.cpp
)

set(HEADERS
    include/SvgIcon.h
    include/SvgIconEngine.h
)

add_library(SvgIconEngine STATIC ${SOURCES} ${HEADERS})
target_link_libraries(SvgIconEngine Qt6::SvgWidgets Qt6::Concurrent)
target_include_directories(SvgIconEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
