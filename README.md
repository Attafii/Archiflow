# ArchiFlow - Complete Architecture Office Management System

## 🏗️ Overview

ArchiFlow is a comprehensive, modern desktop application built with Qt 6.x and C++17 for complete architecture office management. The application provides an integrated platform for managing employees, clients, projects, contracts, materials, and invoicing with advanced features including AI assistance, mapping integration, and automated workflows.

## 🎯 Key Features

### ✨ **Complete Business Management Suite**
- **Employee Management** - Full HR capabilities with location tracking
- **Client Management** - CRM with geolocation and contact management  
- **Project Management** - Comprehensive project lifecycle tracking
- **Contract Management** - Advanced contract handling with AI assistance
- **Materials Management** - Inventory tracking with AI-powered predictions
- **Invoice Management** - Automated billing with PDF generation

### 🤖 **AI-Powered Features**
- **Groq AI Integration** - Advanced natural language processing
- **Smart Predictions** - AI-driven material demand forecasting
- **Automated Documentation** - AI-assisted contract and report generation
- **Intelligent Search** - Context-aware content discovery

### 🗺️ **Location Services**
- **Mapbox Integration** - Professional mapping and geocoding
- **Employee Location Tracking** - Real-time location services
- **Client Address Management** - Geographic visualization
- **Project Site Mapping** - Interactive project location display

## 📱 User Interface Gallery

### Main Dashboard
![Main Dashboard](resources/Screenshot%202025-07-29%20194416.png)

The main dashboard provides a clean, professional interface with:
- **Modular Sidebar Navigation** - Easy access to all features
- **Modern Material Design** - Clean, intuitive interface
- **Status Bar Integration** - Real-time system information
- **Responsive Layout** - Adaptive to different screen sizes

### Employee Management Interface
![Employee Management](resources/Screenshot%202025-07-29%20194505.png)

The employee management module features:
- **Complete Employee Database** - Comprehensive employee records
- **Interactive Table View** - Sortable and filterable employee list
- **Advanced Search Capabilities** - Quick employee lookup
- **Detailed Employee Profiles** - Full contact and position information
- **Location Integration** - Employee address geocoding and mapping
- **Export Functionality** - Data export capabilities

### Materials Management System
![Materials Management](resources/Screenshot%202025-07-29%20194529.png)

The materials management interface includes:
- **Comprehensive Inventory Tracking** - Complete material database
- **Smart Category Management** - Organized material classification
- **Stock Level Monitoring** - Real-time inventory tracking
- **Supplier Integration** - Supplier contact and ordering system
- **AI-Powered Predictions** - Demand forecasting and optimization
- **Cost Analysis Tools** - Material cost tracking and analysis

### Advanced AI Assistant
![AI Assistant](resources/Screenshot%202025-07-29%20194603.png)

The integrated AI assistant provides:
- **Natural Language Processing** - Conversational interface
- **Intelligent Recommendations** - Context-aware suggestions
- **Document Analysis** - AI-powered document processing
- **Predictive Analytics** - Data-driven insights
- **Automated Workflows** - Streamlined business processes
- **Multi-modal Interactions** - Text and document-based queries

## 🏗️ System Architecture

### 📁 Directory Structure

```
ArchiFlow_Application/
├── src/
│   ├── core/                 # Core application framework
│   │   ├── application.h/cpp # Main application singleton
│   │   └── modulemanager.h/cpp # Dynamic module system
│   ├── database/             # Database management layer
│   │   ├── databasemanager.h/cpp # SQLite database handling
│   │   ├── databaseservice.h/cpp # Database service layer
│   │   └── migrations.h/cpp  # Schema migrations
│   ├── ui/                   # User interface framework
│   │   ├── mainwindow.h/cpp/ui # Main application window
│   │   ├── basedialog.h/cpp  # Base dialog foundation
│   │   ├── sidebar.h/cpp     # Navigation sidebar
│   │   └── settingsdialog.h/cpp # Application settings
│   ├── utils/                # Utility services
│   │   ├── environmentloader.h/cpp # Environment configuration
│   │   ├── mapboxhandler.h/cpp # Mapping integration
│   │   ├── stylemanager.h/cpp # UI styling system
│   │   └── documentprocessor.h/cpp # Document processing
│   └── features/             # Feature modules
│       ├── employees/        # Employee management module
│       │   ├── employee.h/cpp # Employee data model
│       │   ├── employeewidget.h/cpp # Employee interface
│       │   ├── employeedialog.h/cpp # Employee forms
│       │   └── employeesmodule.h/cpp # Module integration
│       ├── clients/          # Client management module
│       │   ├── client.h/cpp  # Client data model
│       │   ├── clientwidget.h/cpp # Client interface
│       │   └── clientsmodule.h/cpp # Module integration
│       ├── contracts/        # Contract management module
│       │   ├── contract.h/cpp # Contract data model
│       │   ├── contractwidget.h/cpp # Contract interface
│       │   ├── contractaiassistantdialog.h/cpp # AI assistant
│       │   └── contractmodule.h/cpp # Module integration
│       ├── materials/        # Materials management module
│       │   ├── materialmodel.h/cpp # Material data model
│       │   ├── materialwidget.h/cpp # Materials interface
│       │   ├── aiassistantdialog.h/cpp # AI assistant
│       │   ├── groqclient.h/cpp # AI client integration
│       │   └── materialsmodule.h/cpp # Module integration
│       ├── projects/         # Project management module
│       │   ├── projet.h/cpp  # Project data model
│       │   ├── projetmanager.h/cpp # Project management
│       │   └── projectmodule.h/cpp # Module integration
│       └── invoices/         # Invoice management module
│           ├── invoice.h/cpp # Invoice data model
│           ├── invoicewidget.h/cpp # Invoice interface
│           └── invoicesmodule.h/cpp # Module integration
├── resources/                # Application resources
│   ├── icons/               # Application icons
│   ├── styles/              # Qt stylesheets
│   ├── Screenshot*.png      # Interface screenshots
│   └── resources.qrc        # Qt resource file
├── CMakeLists.txt          # CMake configuration
└── main.cpp               # Application entry point
```

## 🚀 Technical Specifications

### **Core Technologies**
- **Framework**: Qt 6.9.0 with Widgets
- **Language**: C++17 with modern features
- **Database**: SQLite with Qt SQL module
- **Build System**: CMake 3.19+
- **Compiler**: MinGW 64-bit (Windows)

### **External Integrations**
- **Mapbox API**: Professional mapping and geocoding services
- **Groq AI**: Advanced language model integration
- **PDF Generation**: Document export capabilities
- **Environment Configuration**: Secure API key management

### **Architecture Patterns**
- **Modular Design**: Plugin-based feature modules
- **Singleton Pattern**: Application lifecycle management
- **Observer Pattern**: Event-driven communication
- **MVC Architecture**: Clean separation of concerns
- **Factory Pattern**: Dynamic UI component creation

## 🔧 Installation & Setup

### Prerequisites
```bash
# Required Software
- Qt 6.5 or higher
- CMake 3.19 or higher
- C++17 compatible compiler
- Git for version control

# API Keys (Optional)
- Mapbox API key for mapping features
- Groq AI API key for AI assistance
```

### Building the Application

```bash
# Clone the repository
git clone https://github.com/Attafii/Archiflow.git
cd ArchiFlow_Application

# Configure build environment
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=C:/Qt/6.9.0/mingw_64

# Build the application
cmake --build build --config Debug

# Run the application
./build/ArchiFlow_Application.exe
```

### Environment Configuration

Create a `.env` file in the project root:
```bash
# API Configuration
GROQ_API_KEY=your_groq_api_key_here
MAPBOX_API_KEY=pk.your_mapbox_public_token_here
```

## 📊 Feature Modules

### 👥 **Employee Management**
- Complete employee database with personal and professional information
- Position and department management
- Contact information with address geocoding
- Location-based employee mapping
- Advanced search and filtering capabilities
- Export functionality for reports

### 🏢 **Client Management**
- Comprehensive client database and CRM capabilities
- Geographic client distribution mapping
- Contact history and communication tracking
- Project association and management
- Client analytics and reporting

### 📋 **Project Management**
- Full project lifecycle management
- Timeline tracking and milestone management
- Resource allocation and planning
- Project-client-contract integration
- Progress monitoring and reporting

### 📝 **Contract Management**
- Advanced contract creation and management
- AI-powered contract analysis and assistance
- Automated status tracking and alerts
- Template management and customization
- PDF generation and document management

### 📦 **Materials Management**
- Comprehensive inventory tracking system
- Intelligent category and supplier management
- AI-powered demand prediction and optimization
- Cost analysis and budget planning
- Stock level monitoring with automated alerts

### 💰 **Invoice Management**
- Automated invoice generation and calculation
- PDF export with professional templates
- Payment tracking and financial reporting
- Client billing history and analytics
- Integration with project and contract modules

## 🤖 AI Integration

### **Groq AI Assistant**
- Natural language processing for business queries
- Intelligent document analysis and processing
- Automated content generation and recommendations
- Predictive analytics for business optimization
- Context-aware assistance across all modules

### **Smart Features**
- Material demand prediction using historical data
- Automated project timeline estimation
- Intelligent client relationship insights
- Contract risk analysis and recommendations

## 🗺️ Mapping & Location Services

### **Mapbox Integration**
- Professional-grade mapping interface
- Real-time geocoding and address validation
- Employee and client location visualization
- Project site mapping and navigation
- Geographic analytics and reporting

## 🎨 User Interface Design

### **Modern Material Design**
- Clean, professional interface following Material Design principles
- Consistent color scheme and typography
- Responsive layout adapting to different screen sizes
- Intuitive navigation with contextual toolbars
- Accessibility features and keyboard shortcuts

### **Advanced UI Components**
- Interactive data tables with sorting and filtering
- Modern dialog boxes with validation
- Rich text editors for document management
- Chart and graph visualization components
- Drag-and-drop functionality for improved workflows

## 🔒 Security & Configuration

### **API Key Management**
- Secure environment-based configuration
- No hardcoded secrets in source code
- User-configurable API settings
- Encrypted local storage for sensitive data

### **Database Security**
- SQLite with transaction support
- Automated backup and recovery
- Data validation and integrity checks
- User session management

## 📈 Performance & Scalability

### **Optimized Architecture**
- Lazy loading of modules and data
- Efficient database queries with indexing
- Asynchronous operations for smooth UI
- Memory management with smart pointers

### **Cross-Platform Support**
- Windows (primary target)
- macOS compatibility
- Linux support
- Consistent UI across platforms

## 🚀 Current Status

✅ **Fully Implemented**:
- Complete modular architecture
- All six core business modules
- Advanced AI integration
- Professional mapping services
- Modern Material Design UI
- Secure configuration management
- Comprehensive database system

🎯 **Ready for Production**:
- Stable, tested codebase
- Professional user interface
- Complete feature set
- Secure API management
- Comprehensive documentation

---

## 🤝 Contributing

ArchiFlow is designed with a modular architecture that makes it easy to extend and customize. Each module is self-contained and follows consistent patterns for UI, data management, and integration.

### Development Workflow
1. **Module Development**: Each feature is implemented as a separate module
2. **UI Consistency**: All interfaces follow the established design patterns
3. **Database Integration**: Standardized data access patterns
4. **API Integration**: Consistent external service integration
5. **Testing**: Comprehensive testing for stability and reliability

---

**ArchiFlow - Complete Architecture Office Management Solution** 🏗️✨

*A professional, modern application for architecture offices seeking comprehensive business management with advanced AI and mapping capabilities.*
