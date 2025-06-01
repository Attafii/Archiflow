cmake_minimum_required(VERSION 3.19)
project(ContractManagementApp LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql Charts PrintSupport Network)

qt_standard_project_setup()

qt_add_executable(ContractManagementApp
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    statisticsdialog.cpp
    statisticsdialog.h
    statisticsdialog.ui
    databasemanager.cpp
    databasemanager.h
    chatwindow.cpp
    chatwindow.h
    chatbotmanager.cpp
    chatbotmanager.h
    chatbotdialog.cpp
    chatbotdialog.h
    csvimportdialog.cpp
    csvimportdialog.h
)

target_link_libraries(ContractManagementApp
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
        Qt::Charts
        Qt::PrintSupport
        Qt::Network
)

include(GNUInstallDirs)

install(TARGETS ContractManagementApp
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ContractManagementApp
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
