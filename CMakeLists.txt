cmake_minimum_required(VERSION 3.19)

project(client LANGUAGES CXX)

# Find required Qt6 modules
find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebEngineWidgets Sql)

# Set up standard Qt project structure
qt_standard_project_setup()

# Define the executable and its sources
qt_add_executable(client
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    clientmanager.cpp
    clientmanager.h
    clientmanager.ui
    databasemanager.cpp
    databasemanager.h
    mapboxhandler.cpp
    mapboxhandler.h
    groqaiapi.cpp
    groqaiapi.h
    envmanager.cpp
    envmanager.h
)

# Link required Qt6 libraries
target_link_libraries(client
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::WebEngineWidgets
        Qt6::Sql
)

# Set install destinations
include(GNUInstallDirs)

install(TARGETS client
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Generate deployment script for Qt
qt_generate_deploy_app_script(
    TARGET client
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

# Install the generated deployment script
install(SCRIPT ${deploy_script})
