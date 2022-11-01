cmake_minimum_required(VERSION 3.16)

project(qtgstplayer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.2 COMPONENTS Core Gui OpenGL OpenGLWidgets Widgets)
pkg_search_module(egl REQUIRED IMPORTED_TARGET gstreamer-1.0>=1.18)
pkg_search_module(gstreamer REQUIRED IMPORTED_TARGET gstreamer-1.0>=1.18)
# pkg_search_module(gstreamer-app REQUIRED IMPORTED_TARGET gstreamer-app-1.0>=1.18)
pkg_search_module(gstreamer-gl REQUIRED IMPORTED_TARGET gstreamer-gl-1.0>=1.18)
# pkg_search_module(gstreamer-rtsp REQUIRED IMPORTED_TARGET gstreamer-rtsp-1.0>=1.18)
# pkg_search_module(gstreamer-video REQUIRED IMPORTED_TARGET gstreamer-video-1.0>=1.18)

add_executable(qtgstplayer
  source/main.cpp
  source/mainwindow.hpp
  source/mainwindow.cpp
  source/gstplayer/gstplayer.hpp
  source/gstplayer/gstplayer.cpp
  # source/gstplayer/gstthread.cpp
)

target_link_libraries(qtgstplayer
  PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::GuiPrivate
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Widgets
    PkgConfig::gstreamer
    PkgConfig::gstreamer-gl
    # PkgConfig::gstreamer-video
)
