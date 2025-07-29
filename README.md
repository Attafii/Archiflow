# ArchiFlow - Architecture Office Management System

## 🏗️ Project Architecture

ArchiFlow is a modern, modular desktop application built with Qt 6.x and C++17 for architecture office management. The application provides a unified platform for managing employees, clients, projects, contracts, materials, and invoicing.

## 📁 Directory Structure

```
ArchiFlow_Application/
├── src/
│   ├── core/                 # Core application logic
│   │   ├── application.h/cpp # Main application class
│   │   └── modulemanager.h/cpp # Module management system
│   ├── database/             # Database management
│   │   ├── databasemanager.h/cpp # SQLite database handling
│   │   └── migrations.h/cpp  # Schema migrations
│   ├── ui/                   # User interface components
│   │   ├── mainwindow.h/cpp/ui # Main application window
│   │   └── basedialog.h/cpp  # Base dialog class
│   └── features/             # Feature modules (to be implemented)
│       ├── employees/        # Employee management
│       ├── clients/          # Client management
│       ├── contracts/        # Contract management
│       ├── materials/        # Material/stock management
│       ├── projects/         # Project management
│       └── invoices/         # Invoice management
├── resources/                # Application resources
│   ├── icons/               # Application icons
│   ├── styles/              # Qt stylesheets
│   └── resources.qrc        # Qt resource file
├── build/                   # Build output directory
├── old_files/              # Legacy files (temporary)
├── CMakeLists.txt          # CMake configuration
└── main.cpp               # Application entry point
```

## 🎯 Core Features

### 🔧 Architecture Components

1. **Application Class**: Singleton application manager
   - Database initialization
   - Module management
   - Configuration handling
   - Lifecycle management

2. **Module Manager**: Dynamic module system
   - Plugin-like architecture
   - Hot module registration
   - Centralized module lifecycle

3. **Database Manager**: SQLite integration
   - Automatic migrations
   - Transaction support
   - Query optimization
   - Schema versioning

4. **Base UI Classes**: Consistent user interface
   - Modular main window
   - Base dialog templates
   - Modern Qt styling

### 🗄️ Database Schema

- **Core Tables**: settings, audit_log, users, user_sessions
- **Feature Tables**: Will be added as modules are implemented
- **Migration System**: Automatic schema updates
- **Version Control**: Schema versioning for safe updates

## 🚀 Getting Started

### Prerequisites
- Qt 6.5 or higher
- CMake 3.19 or higher
- C++17 compatible compiler
- SQLite (included with Qt)

### Building the Application

```bash
# Configure
cmake -B build -S .

# Build
cmake --build build

# Run
./build/ArchiFlow_Application
```

## 🧩 Module Development

Each feature module should inherit from `BaseModule` and implement:

```cpp
class MyModule : public BaseModule {
public:
    MyModule() : BaseModule("my_module") {}
    
    bool initialize() override;
    void shutdown() override;
    QWidget* createWidget(QWidget *parent = nullptr) override;
};
```

### Module Registration

Modules are automatically registered in `ModuleManager::registerBuiltInModules()`:

```cpp
registerModule(std::make_unique<MyModule>());
```

## 📦 Feature Modules (Planned)

1. **Employees Module**
   - Employee CRUD operations
   - Calendar integration
   - Presence tracking
   - Performance metrics

2. **Clients Module**
   - Client management
   - Geolocation integration
   - Contact history
   - PDF export capabilities

3. **Projects Module**
   - Project lifecycle management
   - Timeline estimation
   - Resource allocation
   - Dashboard visualization

4. **Contracts Module**
   - Contract creation and tracking
   - Automated status updates
   - Alert system
   - PDF generation

5. **Materials Module**
   - Inventory management
   - Stock tracking
   - AI-powered failure prediction
   - Supplier integration

6. **Invoices Module**
   - Automated calculations
   - PDF generation
   - Email integration
   - Payment tracking

## 🎨 User Interface

- **Modern Qt Design**: Clean, professional interface
- **Modular Navigation**: Sidebar-based module switching
- **Responsive Layout**: Splitter-based layout system
- **Consistent Styling**: Custom QSS stylesheets
- **Cross-Platform**: Windows, macOS, and Linux support

## 🔧 Technical Details

- **Framework**: Qt 6.x with Widgets
- **Language**: C++17
- **Database**: SQLite with Qt SQL module
- **Build System**: CMake
- **Architecture**: Modular, plugin-based design
- **Threading**: Qt's signal/slot system for async operations

## 📝 Current Status

✅ **Completed**:
- Core application architecture
- Database management system
- Migration framework
- Base UI infrastructure
- Module management system

🚧 **In Progress**:
- Feature module implementation (awaiting requirements)

📋 **Planned**:
- Individual feature modules
- Advanced UI components
- AI integration
- PDF generation
- Calendar sync
- Map integration

## 🤝 Development Workflow

1. **Base Architecture**: ✅ Complete
2. **Module Implementation**: Ready for feature specifications
3. **Feature Integration**: Per-module implementation
4. **Testing & Polish**: Post-implementation
5. **Deployment**: Final release preparation

---

**Ready for feature module implementation!** 🚀

Each feature will be implemented as a self-contained module with full database integration, modern UI, and seamless integration into the ArchiFlow ecosystem.
