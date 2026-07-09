# The showcase demo. Builds standalone (cmake -S test -B build) or as a
# subdirectory of the top-level project.
cmake_minimum_required(VERSION 3.16)
project(SvgIconEngineDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets SvgWidgets)

# Standalone: pull the library in from the parent directory.
if(NOT TARGET SvgIconEngine::SvgIconEngine)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. SvgIconEngineBuild)
endif()

# Bundle every .svg under svgs/ into a generated resource file, so the demo can
# address icons as ":/icons/regular/heart" without shipping loose files.
# Paths are absolute: relative ones break when this is built as a subdirectory.
set(PATH_TO_SVGS ${CMAKE_CURRENT_SOURCE_DIR}/svgs)
file(GLOB_RECURSE SVG_FILES "${PATH_TO_SVGS}/*.svg")
set(QRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/resources.qrc")
file(WRITE "${QRC_FILE}" "<RCC>\n<qresource prefix=\"/icons\">\n")
foreach(SVG_FILE ${SVG_FILES})
    file(RELATIVE_PATH SVG_FILE_ALIAS "${PATH_TO_SVGS}" "${SVG_FILE}")
    file(APPEND "${QRC_FILE}" "    <file alias=\"${SVG_FILE_ALIAS}\">${SVG_FILE}</file>\n")
endforeach()
file(APPEND "${QRC_FILE}" "</qresource>\n</RCC>\n")

qt_add_resources(RESOURCES "${QRC_FILE}")

add_executable(SvgIconEngineTest main.cpp ${RESOURCES})
target_link_libraries(SvgIconEngineTest PRIVATE SvgIconEngine::SvgIconEngine)
