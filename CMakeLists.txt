cmake_minimum_required(VERSION 3.16)

project(SvgIconEngine
    VERSION 0.2.0
    DESCRIPTION "Tinted, state-aware, animated SVG icons for Qt — as widgets or as a QIcon"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(SVGICONENGINE_BUILD_TESTS "Build the test suite" ${PROJECT_IS_TOP_LEVEL})
option(SVGICONENGINE_BUILD_DEMO  "Build the showcase demo" ${PROJECT_IS_TOP_LEVEL})
option(SVGICONENGINE_BUILD_TOOLS "Build the documentation image generator" OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets SvgWidgets)

set(SOURCES
    src/SvgIcon.cpp
    src/SvgIconEngine.cpp
    src/SvgIconButton.cpp
    src/SvgIconPainter.cpp
    src/SvgQIconEngine.cpp
    src/SvgStroke.cpp
)

# Installed. These are what a consumer includes.
set(PUBLIC_HEADERS
    include/SvgIcon.h
    include/SvgIconEngine.h
    include/SvgIconButton.h
    include/SvgQIconEngine.h
)

# Build-tree only: implementation details the tests reach into, but which no
# public header exposes and no consumer should depend on.
set(INTERNAL_HEADERS
    include/SvgIconPainter.h
    include/SvgStroke.h
)

# STATIC or SHARED follows BUILD_SHARED_LIBS.
add_library(SvgIconEngine ${SOURCES} ${PUBLIC_HEADERS} ${INTERNAL_HEADERS})
add_library(SvgIconEngine::SvgIconEngine ALIAS SvgIconEngine)

target_link_libraries(SvgIconEngine PUBLIC Qt6::Widgets Qt6::SvgWidgets)

set_target_properties(SvgIconEngine PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME SvgIconEngine)

# In-tree consumers include "SvgIcon.h"; installed consumers include
# <SvgIconEngine/SvgIcon.h>. The install interface therefore points at the
# include root, not at the package subdirectory — otherwise the prefixed form
# cannot resolve. Headers include each other relatively, so both work.
target_include_directories(SvgIconEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# ── install ────────────────────────────────────────────────────────────────
install(TARGETS SvgIconEngine
    EXPORT SvgIconEngineTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ${PUBLIC_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SvgIconEngine)

install(EXPORT SvgIconEngineTargets
    FILE      SvgIconEngineTargets.cmake
    NAMESPACE SvgIconEngine::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SvgIconEngine)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/SvgIconEngineConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/SvgIconEngineConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SvgIconEngine)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/SvgIconEngineConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/SvgIconEngineConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/SvgIconEngineConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SvgIconEngine)

# ── tests & demo ───────────────────────────────────────────────────────────
if(SVGICONENGINE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(SVGICONENGINE_BUILD_DEMO)
    add_subdirectory(test)
endif()

if(SVGICONENGINE_BUILD_TOOLS)
    add_subdirectory(tools)
endif()
