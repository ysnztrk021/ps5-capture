cmake_minimum_required(VERSION 3.16)
project(ps5_capture LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-video-1.0
    gstreamer-app-1.0
)

qt_add_resources(APP_RESOURCES resources.qrc)

add_executable(ps5_capture
    main.cpp
    mainwindow.cpp
    gstwidget.cpp
    ${APP_RESOURCES}
)

target_include_directories(ps5_capture PRIVATE ${GST_INCLUDE_DIRS})
target_link_libraries(ps5_capture
    Qt6::Widgets
    ${GST_LIBRARIES}
)