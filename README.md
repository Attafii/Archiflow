# ContractManagementApp

This is a desktop application for managing contracts, built with C++ and the Qt framework. It provides functionalities for storing, retrieving, and analyzing contract data, along with an integrated chatbot for assistance.

## Features

*   **Contract Data Management:** Add, view, edit, and delete contracts.
*   **Database Storage:** Securely stores contract information in a local database.
*   **Chatbot Assistant:** An integrated chatbot to help users navigate the application or query contract details.
*   **Statistics View:** Display relevant statistics and insights from the contract data.
*   **User-Friendly Interface:** Intuitive GUI built with Qt for easy interaction.

## Tech Stack

*   **Programming Language:** C++
*   **Framework:** Qt 6.9.0
*   **Build System:** CMake
*   **Compiler:** MinGW (as per build directory)
*   **Database:** SQLite (inferred from `contracts.db` and `databasemanager.cpp`)

## Build Instructions

### Prerequisites

*   Qt 6.9.0 or later (with MinGW toolchain if on Windows)
*   CMake 3.16 or later
*   A C++ compiler compatible with C++17 (or as specified in CMakeLists.txt)

### Steps

1.  **Clone the repository (if applicable):**
    ```bash
    git clone <repository-url>
    cd ContractManagementApp
    ```
2.  **Configure CMake:**
    Open a terminal or command prompt in the project's root directory.
    ```bash
    mkdir build
    cd build
    cmake .. -G "MinGW Makefiles" # Or your preferred generator
    ```
    (If using Qt Creator, it can often handle CMake configuration automatically when opening `CMakeLists.txt`)

3.  **Build the project:**
    ```bash
    cmake --build .
    ```
    Or, using MinGW directly:
    ```bash
    mingw32-make
    ```

## Usage

1.  After a successful build, the executable (`ContractManagementApp.exe` on Windows) will be located in the `build/Desktop_Qt_6_9_0_MinGW_64_bit-Debug/` directory (or a similar path depending on your build configuration).
2.  Run the executable to start the application.
3.  Use the main window to manage contracts, interact with the chatbot, and view statistics.

## Project Structure

*   `chatbotmanager.cpp/.h`: Manages the chatbot logic.
*   `chatwindow.cpp/.h`: UI for the chatbot.
*   `databasemanager.cpp/.h`: Handles database interactions for contracts.
*   `main.cpp`: Entry point of the application.
*   `mainwindow.cpp/.h/.ui`: Implements the main application window and its UI.
*   `statisticsdialog.cpp/.h/.ui`: Implements the dialog for displaying statistics.
*   `CMakeLists.txt`: CMake build script.
*   `contracts.db`: SQLite database file (created/used at runtime).
*   `tests/`: Contains unit tests (e.g., `test_databasemanager.cpp`).
*   `build/`: Contains build artifacts.

## Contributing

[Details on how to contribute to the project, if open for contributions. e.g., coding standards, pull request process.]

## License

[Specify the license for the project, e.g., MIT, GPL, etc.]
