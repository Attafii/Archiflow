# Material Management System

A Qt 6 C++ application for managing materials inventory with a modern dark UI theme.

## Project Overview

This Material Management System follows a clean, modular architecture using the Model-View-Controller (MVC) pattern. It provides functionality for material management, data visualization, and CSV import/export.

## Features

- **Material Management**: CRUD operations for materials (name, quantity, unit, price, description)
- **Search and Filter**: Find materials by name, quantity, or price
- **Data Visualization**: Bar charts for inventory levels and pie charts for value distribution
- **CSV Import/Export**: Import and export material data
- **Modern UI**: Dark-themed, responsive interface

## Project Structure

```
/
├── controllers/    # Controller classes for business logic
├── database/       # Database management and SQL operations
├── models/         # Data models and QSqlTableModel implementations
├── ui/             # UI forms and view components
├── utils/          # Utility classes and helper functions
```

## Technical Details

- **Framework**: Qt 6 with Widgets, SQL, and Charts modules
- **Database**: SQLite for local data storage
- **Architecture**: Model-View-Controller (MVC) pattern
- **C++ Standard**: C++17

## Building the Project

### Prerequisites

- Qt 6.5 or higher
- CMake 3.19 or higher
- C++17 compatible compiler

### Build Instructions

1. Clone or download the repository
2. Open a terminal/command prompt in the project directory
3. Create a build directory and navigate to it:

```bash
mkdir build
cd build
```

4. Run CMake to configure the project:

```bash
cmake ..
```

5. Build the project:

```bash
cmake --build .
```

6. Run the application:

```bash
# On Windows
Debug\Materials.exe

# On macOS/Linux
./Materials
```

## Development Phases

### Phase 1: Project Setup ✅
- Created Qt 6 Widgets Application
- Set up folder structure
- Initialized SQLite database with materials table
- Added dark UI stylesheet

### Phase 2: Material Management 🔄
- Create QTableView with QSqlTableModel
- Add form to add/edit/delete material
- Implement filter/search fields

### Phase 3: CSV Import/Export 🔄
- Implement CSV import using QFile and QTextStream
- Implement CSV export

### Phase 4: Data Visualization 🔄
- Use QtCharts to display bar and pie charts

### Phase 5: UI Polish 🔄
- Refactor layout and navigation
- Apply consistent dark style to all widgets