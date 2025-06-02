cmake_minimum_required(VERSION 3.16)

project(InvoiceApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Gui Core)

add_executable(InvoiceApp
    main.cpp
    mainwindow.h
    mainwindow.cpp
)

target_link_libraries(InvoiceApp PRIVATE Qt6::Widgets Qt6::Sql Qt6::Gui Qt6::Core)

install(TARGETS InvoiceApp
    RUNTIME DESTINATION bin
)

