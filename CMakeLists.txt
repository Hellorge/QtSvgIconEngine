cmake_minimum_required(VERSION 3.5)

project(SvgIconEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg SvgWidgets Concurrent)

file(GLOB_RECURSE SVG_FILES "${CMAKE_SOURCE_DIR}/svgs/*.svg")
file(WRITE "${CMAKE_BINARY_DIR}/resources.qrc" "<RCC>\n<qresource prefix=\"/icons\">\n")
foreach(SVG_FILE ${SVG_FILES})
    file(RELATIVE_PATH SVG_FILE_ALIAS "${CMAKE_SOURCE_DIR}/svgs" "${SVG_FILE}")
    file(RELATIVE_PATH SVG_FILE_REL "${CMAKE_BINARY_DIR}" "${SVG_FILE}")
    file(APPEND "${CMAKE_BINARY_DIR}/resources.qrc" "    <file alias=\"${SVG_FILE_ALIAS}\">${SVG_FILE_REL}</file>\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/resources.qrc" "</qresource>\n</RCC>\n")

qt_add_resources(RESOURCES "${CMAKE_BINARY_DIR}/resources.qrc")

set(SOURCES
    src/SvgIcon.cpp
    src/SvgIconEngine.cpp
    main.cpp
)

set(HEADERS
    include/SvgIcon.h
    include/SvgIconEngine.h
)

add_library(SvgIconEngine STATIC ${SOURCES} ${HEADERS})
target_link_libraries(SvgIconEngine Qt6::Widgets Qt6::Svg Qt6::SvgWidgets Qt6::Concurrent)
include_directories(SvgIconEngine PRIVATE include)

add_executable(icon_engine_test main.cpp ${RESOURCES})
target_link_libraries(icon_engine_test PRIVATE SvgIconEngine)
