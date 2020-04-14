project(examples)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND CMAKE_PREFIX_PATH "/usr/local")

if (DEFINED ENV{QT_ROOT})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QT_ROOT}")
endif ()

message("--------" "${CMAKE_PREFIX_PATH}")

find_package(Qt5 COMPONENTS Widgets REQUIRED)

aux_source_directory(. DIR_SRCS)

add_executable(${PROJECT_NAME}
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Widgets
        smt-baumer)