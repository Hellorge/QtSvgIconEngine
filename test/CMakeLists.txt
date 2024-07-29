cmake_minimum_required(VERSION 3.10)
project(SvgIconEngineTest)

# Specify the minimum required C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

set(PATH_TO_SVGS ${CMAKE_SOURCE_DIR}/svgs)
file(GLOB_RECURSE SVG_FILES "${PATH_TO_SVGS}/*.svg")
file(WRITE "${CMAKE_BINARY_DIR}/resources.qrc" "<RCC>\n<qresource prefix=\"/icons\">\n")
foreach(SVG_FILE ${SVG_FILES})
    file(RELATIVE_PATH SVG_FILE_ALIAS "${PATH_TO_SVGS}" "${SVG_FILE}")
    file(RELATIVE_PATH SVG_FILE_REL "${CMAKE_BINARY_DIR}" "${SVG_FILE}")
    file(APPEND "${CMAKE_BINARY_DIR}/resources.qrc" "    <file alias=\"${SVG_FILE_ALIAS}\">${SVG_FILE_REL}</file>\n")
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/resources.qrc" "</qresource>\n</RCC>\n")


qt_add_resources(RESOURCES "${CMAKE_BINARY_DIR}/resources.qrc")


add_executable(SvgIconEngineTest main.cpp ${RESOURCES})

set(SVGICONENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
add_subdirectory(${SVGICONENGINE_DIR} SvgIconEngineBuild)

target_link_libraries(SvgIconEngineTest PRIVATE SvgIconEngine)
