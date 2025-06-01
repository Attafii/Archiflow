cmake_minimum_required(VERSION 3.16)

project(GestionDesProjets VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Recherche des composants Qt de base (toujours requis)
find_package(Qt6 COMPONENTS Core Gui Widgets Network REQUIRED)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/projet.cpp
    src/projetmanager.cpp
    src/projetdialog.cpp
    src/durationestimator.cpp
    src/searchfilter.cpp
    include/mainwindow.h
    include/projet.h
    include/projetmanager.h
    include/projetdialog.h
    include/durationestimator.h
    include/searchfilter.h
    forms/mainwindow.ui
    forms/projetdialog.ui
    forms/searchfilter.ui
    resources/resources.qrc
)

add_executable(GestionDesProjets ${PROJECT_SOURCES}
    include/Arduino.h
    src/Arduino.cpp
)

target_include_directories(GestionDesProjets PRIVATE include)

# Lier les bibliothèques Qt de base
target_link_libraries(GestionDesProjets PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

install(TARGETS GestionDesProjets
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Ajout des instructions pour Qt Creator
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(GestionDesProjets PRIVATE DEBUG_BUILD)
endif()

# Définir le répertoire de sortie
set_target_properties(GestionDesProjets PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
