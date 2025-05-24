cmake_minimum_required(VERSION 3.19)
project(Materials LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql Charts PrintSupport)

qt_standard_project_setup()

qt_add_executable(Materials
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    materialmodel.cpp
    materialmodel.h
    QRangeSlider.cpp
    QRangeSlider.h
    database/databasemanager.cpp
    database/databasemanager.h
    resources.qrc
)

target_link_libraries(Materials
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
        Qt::Charts
        Qt::PrintSupport
)

# Add the current directory to the include path
target_include_directories(Materials PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(GNUInstallDirs)

install(TARGETS Materials
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Materials
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
