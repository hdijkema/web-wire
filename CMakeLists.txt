cmake_minimum_required(VERSION 3.16)

project(web-wire LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS HttpServer)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(web-wire
  main.cpp
  webwirehandler.h webwirehandler.cpp
  consolelistener.h consolelistener.cpp
  readlineinthread.h readlineinthread.cpp
  webwirewindow.h webwirewindow.cpp
  webwirepage.h webwirepage.cpp
  webwireview.h webwireview.cpp
  webwireprofile.h webwireprofile.cpp
  execjs.h execjs.cpp
  webwire.h
  default_css.h
  default_css.cpp
  devtoolswindow.h devtoolswindow.cpp
)

target_link_libraries(web-wire Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(web-wire Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(web-wire Qt${QT_VERSION_MAJOR}::WebEngineWidgets)
target_link_libraries(web-wire Qt${QT_VERSION_MAJOR}::HttpServer)

include(GNUInstallDirs)
install(TARGETS web-wire
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
