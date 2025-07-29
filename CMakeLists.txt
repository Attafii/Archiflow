cmake_minimum_required(VERSION 3.19)
project(ArchiFlow_Application LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

# Set the possible values of build type for cmake-gui
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

# Set Qt6 path for different scenarios
if(WIN32)
    # Set Qt6 installation path
    if(NOT DEFINED CMAKE_PREFIX_PATH)
        set(CMAKE_PREFIX_PATH "C:/Qt/6.9.0/mingw_64")
    endif()
    
    # Add MinGW to PATH if available
    if(EXISTS "C:/Qt/Tools/mingw1310_64/bin")
        set(CMAKE_MAKE_PROGRAM "C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe" CACHE FILEPATH "MinGW Make")
        set(CMAKE_C_COMPILER "C:/Qt/Tools/mingw1310_64/bin/gcc.exe" CACHE FILEPATH "MinGW C Compiler")
        set(CMAKE_CXX_COMPILER "C:/Qt/Tools/mingw1310_64/bin/g++.exe" CACHE FILEPATH "MinGW C++ Compiler")
    endif()
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core 
    Widgets 
    Sql 
    Charts 
    PrintSupport 
    Network
    Concurrent
    Test
)

# Compiler-specific flags for better warnings
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

qt_standard_project_setup()

# Include directories
include_directories(src)

# Core files
set(CORE_SOURCES
    src/core/application.cpp
    src/core/application.h
    src/core/modulemanager.cpp
    src/core/modulemanager.h
)

# Database files
set(DATABASE_SOURCES
    src/database/databasemanager.cpp
    src/database/databasemanager.h
    src/database/migrations.cpp
    src/database/migrations.h
    src/database/databaseservice.cpp
    src/database/databaseservice.h
)

# Interfaces
set(INTERFACE_SOURCES
    src/interfaces/idatabasemanager.h
    src/interfaces/imoduleinterface.h
    src/interfaces/icontractservice.h
    src/interfaces/icontractwidget.h
    src/interfaces/icontractdialog.h
    src/interfaces/icontractexporter.h
    src/interfaces/icontractchatbot.h
    src/interfaces/icontractimporter.h
    src/interfaces/interfaces.h
)

# Contract Management Module
set(CONTRACTS_SOURCES
    src/features/contracts/contract.cpp
    src/features/contracts/contract.h
    src/features/contracts/contractdatabasemanager.cpp
    src/features/contracts/contractdatabasemanager.h
    src/features/contracts/contractwidget.cpp
    src/features/contracts/contractwidget.h
    src/features/contracts/contractdialog.cpp    src/features/contracts/contractdialog.h
    src/features/contracts/contractmodule.cpp
    src/features/contracts/contractmodule.h
    src/features/contracts/contractstatisticswidget.cpp
    src/features/contracts/contractstatisticswidget.h
    src/features/contracts/contractexportmanager.cpp
    src/features/contracts/contractexportmanager.h
    src/features/contracts/contractchatbotdialog.cpp
    src/features/contracts/contractchatbotdialog.h
    src/features/contracts/contractimportdialog.cpp
    src/features/contracts/contractimportdialog.h
    src/features/contracts/groqcontractchatbot.cpp
    src/features/contracts/groqcontractchatbot.h
    src/features/contracts/contractaiassistantdialog.cpp
    src/features/contracts/contractaiassistantdialog.h
)

# UI files
set(UI_SOURCES
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/mainwindow.ui
    src/ui/basedialog.cpp
    src/ui/basedialog.h
    src/ui/sidebar.cpp
    src/ui/sidebar.h
    src/ui/settingsdialog.cpp
    src/ui/settingsdialog.h
)

# Materials Management Module
set(MATERIALS_SOURCES
    src/features/materials/materialmodel.cpp
    src/features/materials/materialmodel.h
    src/features/materials/materialwidget.cpp
    src/features/materials/materialwidget.h
    src/features/materials/materialdialog.cpp
    src/features/materials/materialdialog.h
    src/features/materials/materialdetailsdialog.cpp
    src/features/materials/materialdetailsdialog.h
    src/features/materials/materialsmodule.cpp
    src/features/materials/materialsmodule.h
    src/features/materials/groqclient.cpp
    src/features/materials/groqclient.h
    src/features/materials/aiassistantdialog.cpp
    src/features/materials/aiassistantdialog.h
    src/features/materials/aipredictiondialog.cpp
    src/features/materials/aipredictiondialog.h
    src/features/materials/suppliermodel.cpp
    src/features/materials/suppliermodel.h
    src/features/materials/supplierwidget.cpp
    src/features/materials/supplierwidget.h
    src/features/materials/supplierdialog.cpp
    src/features/materials/supplierdialog.h
)

# Project Management Module
set(PROJECTS_SOURCES
    src/features/projects/projet.cpp
    src/features/projects/projet.h
    src/features/projects/projetmanager.cpp
    src/features/projects/projetmanager.h
    src/features/projects/projectmodule.cpp
    src/features/projects/projectmodule.h
    src/ui/projects/projetdialog.cpp
    src/ui/projects/projetdialog.h
    src/ui/projects/projetdialog.ui
    src/ui/projects/projetwidget.cpp
    src/ui/projects/projetwidget.h
)

# Invoice Management Module
set(INVOICES_SOURCES
    src/features/invoices/invoice.cpp
    src/features/invoices/invoice.h
    src/features/invoices/invoiceitem.cpp
    src/features/invoices/invoiceitem.h
    src/features/invoices/client.cpp
    src/features/invoices/client.h
    src/features/invoices/invoicedatabasemanager.cpp
    src/features/invoices/invoicedatabasemanager.h
    src/features/invoices/invoicewidget.cpp
    src/features/invoices/invoicewidget.h
    src/features/invoices/invoicedialog.cpp
    src/features/invoices/invoicedialog.h
    src/features/invoices/clientdialog.cpp
    src/features/invoices/clientdialog.h
    src/features/invoices/invoicepdfgenerator.cpp
    src/features/invoices/invoicepdfgenerator.h
    src/features/invoices/invoiceaiassistantdialog.cpp
    src/features/invoices/invoiceaiassistantdialog.h
    src/features/invoices/invoicesmodule.cpp
    src/features/invoices/invoicesmodule.h
)

# Client Management Module
set(CLIENTS_SOURCES
    src/features/clients/client.cpp # Renamed from client.cpp
    src/features/clients/client.h   # Renamed from client.h
    src/features/clients/clientdatabasemanager.cpp
    src/features/clients/clientdatabasemanager.h
    src/features/clients/clientwidget.cpp
    src/features/clients/clientwidget.h
    src/features/clients/clientdialog.cpp
    src/features/clients/clientdialog.h
    src/features/clients/clientsmodule.cpp
    src/features/clients/clientsmodule.h
    src/features/clients/clientaiassistant.cpp
    src/features/clients/clientaiassistant.h
)

# Employee Management Module
set(EMPLOYEES_SOURCES
    src/features/employees/employee.cpp
    src/features/employees/employee.h
    src/features/employees/employeedatabasemanager.cpp
    src/features/employees/employeedatabasemanager.h
    src/features/employees/employeewidget.cpp
    src/features/employees/employeewidget.h
    src/features/employees/employeedialog.cpp
    src/features/employees/employeedialog.h
    src/features/employees/employeesmodule.cpp
    src/features/employees/employeesmodule.h
)

# Utility files
set(UTILS_SOURCES
    src/utils/environmentloader.cpp
    src/utils/environmentloader.h
    src/utils/mapboxhandler.cpp
    src/utils/mapboxhandler.h
    src/utils/stylemanager.cpp
    src/utils/stylemanager.h
    src/utils/animationmanager.cpp
    src/utils/animationmanager.h
    src/utils/documentprocessor.cpp
    src/utils/documentprocessor.h
)

# Source files for the main application
set(APPLICATION_SOURCES
    main.cpp
    ${CORE_SOURCES}
    ${DATABASE_SOURCES}
    ${INTERFACE_SOURCES}
    ${UI_SOURCES}
    ${CONTRACTS_SOURCES}
    ${MATERIALS_SOURCES}
    ${PROJECTS_SOURCES}
    ${INVOICES_SOURCES}
    ${CLIENTS_SOURCES}         # Added CLIENTS_SOURCES
    ${EMPLOYEES_SOURCES}       # Added EMPLOYEES_SOURCES
    ${UTILS_SOURCES}           # Added UTILS_SOURCES
)

qt_add_executable(ArchiFlow_Application WIN32
    ${APPLICATION_SOURCES}
)

# Resources
qt_add_resources(ArchiFlow_Application "app_resources"
    PREFIX "/"
    FILES
        resources/styles/main.qss
)

qt_add_resources(ArchiFlow_Application "app_icons"
    PREFIX "/icons"
    FILES
        resources/icons/ArchiFlow.svg
        resources/icons/ArchiFlow.png
        resources/icons/a.png
)

# Link libraries
target_link_libraries(ArchiFlow_Application PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Charts    Qt6::PrintSupport
    Qt6::Network
    Qt6::Concurrent
    Qt6::Test
)

# Copy .env file to build directory for development
add_custom_command(TARGET ArchiFlow_Application POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/.env"
        "$<TARGET_FILE_DIR:ArchiFlow_Application>/.env"
    COMMENT "Copying .env file to build directory"
)

include(GNUInstallDirs)

install(TARGETS ArchiFlow_Application
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Enable testing
enable_testing()

# Create test executable for Contract CRUD operations
qt_add_executable(test_contract_crud
    test_contract_crud.cpp
)

# Add sources that need to be tested
target_sources(test_contract_crud PRIVATE
    src/features/contracts/contract.cpp
    src/features/contracts/contractdatabasemanager.cpp
    src/database/databasemanager.cpp
    src/database/migrations.cpp
    src/utils/environmentloader.cpp
)

# Link required libraries for the test
target_link_libraries(test_contract_crud
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
        Qt::Charts
        Qt::Test
)

# Include test directories
target_include_directories(test_contract_crud PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add test to CTest
add_test(NAME ContractCRUDTest COMMAND test_contract_crud)

# Set test properties
set_tests_properties(ContractCRUDTest PROPERTIES
    TIMEOUT 300
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Create simple test executable for Contract CRUD operations
qt_add_executable(test_simple_crud
    test_simple_crud.cpp
    src/features/contracts/contract.cpp
    src/features/contracts/contractdatabasemanager.cpp
)

# Link required libraries for the simple test
target_link_libraries(test_simple_crud
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
)

# Include directories for the simple test
target_include_directories(test_simple_crud PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add simple test to CTest
add_test(NAME SimpleCRUDTest COMMAND test_simple_crud)

# Set test properties
set_tests_properties(SimpleCRUDTest PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Employee Core Test
qt_add_executable(test_employee_core
    test_employee_core.cpp
    src/features/employees/employee.cpp
    src/features/employees/employeedatabasemanager.cpp
)

target_link_libraries(test_employee_core PRIVATE
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Sql
    Qt6::Test
)

target_include_directories(test_employee_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add employee test to CTest
add_test(NAME EmployeeCoreTest COMMAND test_employee_core)

# Set test properties
set_tests_properties(EmployeeCoreTest PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Employee Integration Test
qt_add_executable(test_integration
    test_integration.cpp
    ${SOURCES}
    ${MATERIALS_SOURCES}
    ${PROJECTS_SOURCES}
    ${CONTRACTS_SOURCES}
    ${INVOICE_SOURCES}
    ${CLIENTS_SOURCES}
    ${EMPLOYEES_SOURCES}
)

target_link_libraries(test_integration PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Charts
    Qt6::PrintSupport
    Qt6::Network
)

target_include_directories(test_integration PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add integration test to CTest
add_test(NAME EmployeeIntegrationTest COMMAND test_integration)

# Set test properties
set_tests_properties(EmployeeIntegrationTest PROPERTIES
    TIMEOUT 120
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

qt_generate_deploy_app_script(
    TARGET ArchiFlow_Application
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
