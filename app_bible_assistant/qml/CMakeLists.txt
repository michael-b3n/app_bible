cmake_minimum_required(VERSION 3.30)

find_package(${QT_VERSION_NAME} ${QT_VERSION_NUMBER} REQUIRED COMPONENTS Qml)

qt_add_library(main_ui STATIC)
qt_add_qml_module(
  main_ui
  URI "MainUI"
  NO_PLUGIN
  QML_FILES
  Main.qml
  SOURCES
  RESOURCES
)
