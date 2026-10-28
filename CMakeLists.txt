cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Graphic VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(appGraphic

)

qt_add_qml_module(appGraphic
    URI Graphic
    VERSION 1.0
    QML_FILES
    SOURCES main.cpp
    SOURCES pixel_buffer.h
    SOURCES rendertarget.h rendertarget.cpp
    SOURCES canvas.h canvas.cpp
    SOURCES ui/canvaswidget.h ui/canvaswidget.cpp
    SOURCES camera.h camera.cpp
    SOURCES pixel.h pixel.cpp
    SOURCES point.h point.cpp
    SOURCES drawable/drawable.h 
    SOURCES drawable/line.h drawable/line.cpp
    SOURCES ui/mainwindow.h ui/mainwindow.cpp
    SOURCES tool.h
    SOURCES rgba.h
    SOURCES drawable/circle.h drawable/circle.cpp
    SOURCES ui/colorselection.h ui/colorselection.cpp
    SOURCES drawable/polygon.h drawable/polygon.cpp
    SOURCES linetype.h linetype.cpp
    SOURCES geometry.h geometry.cpp
    SOURCES transform.h transform.cpp
    SOURCES ui/transformdialog.h ui/transformdialog.cpp
    SOURCES aabb.h aabb.cpp
    SOURCES drawable/bspline.h drawable/bspline.cpp
    SOURCES drawable/beziercurve.h drawable/beziercurve.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appGraphic PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appGraphic
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appGraphic
    PRIVATE Qt6::Quick)
target_link_libraries(appGraphic
    PRIVATE Qt6::Widgets)

include(GNUInstallDirs)
install(TARGETS appGraphic
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)