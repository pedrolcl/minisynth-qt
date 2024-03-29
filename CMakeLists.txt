cmake_minimum_required(VERSION 3.5)

project(minisynth-qt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 Qt6)
if ((CMAKE_SYSTEM_NAME MATCHES "Linux") AND (QT_VERSION_MAJOR EQUAL 6) AND (QT_VERSION VERSION_LESS 6.4))
    message(WARNING "Unsupported Qt version ${QT_VERSION} for system ${CMAKE_SYSTEM_NAME}")
endif()
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Multimedia REQUIRED)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    tonesynth.h
    tonesynth.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(minisynth-qt
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(minisynth-qt
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(minisynth-qt PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
)

if (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    target_link_options(minisynth-qt PUBLIC -sASYNCIFY -Os)
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_target(minisynth-qt)
endif()
