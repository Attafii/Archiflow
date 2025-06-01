# Employee Manager Lite

A modern, secure employee management system built with Qt6 and C++. This application provides a comprehensive solution for managing employee data with Google OAuth integration for secure authentication.

## 🚀 Features

- **Employee Management**: Complete CRUD operations for employee records
- **Secure Authentication**: Google OAuth 2.0 integration with environment-based credential management
- **Modern UI**: Clean and intuitive Qt6-based interface
- **Database Integration**: SQLite database for reliable data storage
- **Search & Filter**: Advanced employee search and filtering capabilities
- **Data Export**: Export employee data to various formats
- **Security-First**: No hardcoded credentials, proper secret management

## 🛠️ Technologies Used

- **Frontend**: Qt6 with C++
- **Database**: SQLite
- **Authentication**: Google OAuth 2.0
- **Build System**: qmake
- **Compiler**: MinGW 64-bit

## 📋 Prerequisites

Before running this application, ensure you have:

- Qt6 (6.7.3 or later)
- MinGW 64-bit compiler
- Git
- Google Cloud Platform account (for OAuth setup)

## 🔧 Installation & Setup

### 1. Clone the Repository

```bash
git clone https://github.com/Attafii/Archiflow.git
cd EmployeeManagerLite
```

### 2. Google OAuth Setup

1. Go to the [Google Cloud Console](https://console.cloud.google.com/)
2. Create a new project or select an existing one
3. Enable the Google+ API
4. Create OAuth 2.0 credentials (Desktop Application)
5. Copy your Client ID and Client Secret

### 3. Environment Configuration

1. Copy the example environment file:
```bash
copy .env.example .env
```

2. Edit `.env` and add your Google OAuth credentials:
```env
GOOGLE_CLIENT_ID=your-google-client-id-here
GOOGLE_CLIENT_SECRET=your-google-client-secret-here
```

3. **Important**: Never commit the `.env` file to version control!

### 4. Build the Application

#### Using Qt Creator:
1. Open `EmployeeManagerLite.pro` in Qt Creator
2. Configure the project with MinGW 64-bit kit
3. Build and run

#### Using Command Line:
```bash
qmake EmployeeManagerLite.pro
make
```

## 🚀 Running the Application

### Set Environment Variables (Windows)

Before running the application, set your environment variables:

```powershell
$env:GOOGLE_CLIENT_ID="your-google-client-id-here"
$env:GOOGLE_CLIENT_SECRET="your-google-client-secret-here"
./EmployeeManagerLite.exe
```

### Alternative: Using .env file

The application automatically loads environment variables from a `.env` file if present in the application directory.

## 📁 Project Structure

```
EmployeeManagerLite/
├── main.cpp                 # Application entry point
├── mainwindow.cpp          # Main window implementation
├── mainwindow.h            # Main window header
├── resources.qrc           # Qt resource file
├── EmployeeManagerLite.pro # Qt project file
├── .gitignore             # Git ignore rules
├── .env.example           # Environment variables template
├── SETUP.md              # Detailed setup instructions
├── README.md             # This file
└── icons/                # Application icons
    ├── add.png
    ├── delete.png
    ├── reset.png
    ├── search.png
    ├── sync.png
    └── update.png
```

## 🔒 Security Features

This application implements several security best practices:

- **Environment-based Configuration**: OAuth credentials are loaded from environment variables
- **No Hardcoded Secrets**: All sensitive data is externalized
- **Git Security**: Comprehensive `.gitignore` prevents accidental credential commits
- **Build Artifact Exclusion**: Compiled files are excluded from version control

## 📱 User Interface

The application features a modern Qt6 interface with:

- **Employee List View**: Displays all employees in a table format
- **Add/Edit Forms**: User-friendly forms for employee data entry
- **Search Functionality**: Real-time employee search
- **Action Buttons**: Quick access to common operations
- **Status Indicators**: Visual feedback for operations

## 🗄️ Database Schema

The application uses SQLite with the following main tables:

- **employees**: Core employee information
  - id (Primary Key)
  - name
  - email
  - department
  - position
  - hire_date
  - salary
  - created_at
  - updated_at

## 🔧 Configuration

### Environment Variables

| Variable | Description | Required |
|----------|-------------|----------|
| `GOOGLE_CLIENT_ID` | Google OAuth Client ID | Yes |
| `GOOGLE_CLIENT_SECRET` | Google OAuth Client Secret | Yes |

### Application Settings

The application stores user preferences and settings in the system's standard location for application data.

## 🚨 Troubleshooting

### Common Issues

1. **OAuth Authentication Fails**
   - Verify your Google OAuth credentials are correct
   - Ensure environment variables are properly set
   - Check that the Google+ API is enabled in your project

2. **Database Connection Issues**
   - Ensure the application has write permissions in its directory
   - Check that SQLite driver is available

3. **Build Errors**
   - Verify Qt6 is properly installed
   - Ensure MinGW 64-bit compiler is configured
   - Check that all dependencies are available

### Debug Mode

To run in debug mode with verbose logging:

```bash
set QT_LOGGING_RULES="*.debug=true"
./EmployeeManagerLite.exe
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Code Style

- Follow Qt coding conventions
- Use meaningful variable and function names
- Comment complex logic
- Ensure all new features have appropriate error handling

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**Ahmed Attafi**
- GitHub: [@Attafii](https://github.com/Attafii)

## 🙏 Acknowledgments

- Qt Framework for the excellent GUI toolkit
- Google for OAuth 2.0 services
- SQLite for the reliable database engine

## 📞 Support

If you encounter any issues or have questions:

1. Check the [Issues](https://github.com/Attafii/Archiflow/issues) page
2. Create a new issue if your problem isn't already reported
3. Provide detailed information about your environment and the issue

## 🔄 Version History

- **v1.0.0** - Initial release with secure OAuth implementation
  - Employee CRUD operations
  - Google OAuth integration
  - SQLite database support
  - Modern Qt6 interface

---

⚠️ **Security Notice**: This application handles sensitive employee data. Always ensure you're following your organization's data protection policies and applicable privacy laws when using this software.
