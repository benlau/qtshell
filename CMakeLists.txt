cmake_minimum_required(VERSION 3.10)
project(qtshell)

find_package(Qt5 COMPONENTS Core REQUIRED)
set(SOURCES
    ./priv/qtshellrealpath.cpp
    ./priv/qtshellpriv.cpp
    ./priv/qtshellmv.cpp
    ./qtshell.cpp)
add_library(qtshell ${SOURCES})

target_include_directories(qtshell PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

target_link_libraries(qtshell PRIVATE Qt5::Core)
set_target_properties(qtshell PROPERTIES PUBLIC_HEADER "qtshell.h;QtShell")

install(TARGETS qtshell
    EXPORT qtshellTargets
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})
export(TARGETS qtshell NAMESPACE qtshell:: FILE qtshellConfig.cmake)
set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
export(PACKAGE qtshell)
install(EXPORT qtshellTargets NAMESPACE qtshell:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qtshell FILE qtshellConfig.cmake)
