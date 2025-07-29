#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QSettings>
#include <memory>

class DatabaseManager;
class ModuleManager;

/**
 * @brief The Application class - Core application singleton
 * 
 * This class manages the overall application lifecycle, configuration,
 * and provides access to core services like database and module management.
 */
class Application : public QApplication
{
    Q_OBJECT

public:
    explicit Application(int &argc, char **argv);
    ~Application();

    // Singleton access
    static Application* instance();

    // Core services
    DatabaseManager* databaseManager() const;
    ModuleManager* moduleManager() const;
    QSettings* settings() const;

    // Application lifecycle
    bool initialize();
    void shutdown();

    // Configuration
    QString applicationDataPath() const;
    QString databasePath() const;

signals:
    void applicationInitialized();
    void applicationShuttingDown();

private slots:
    void onDatabaseConnected();
    void onDatabaseError(const QString &error);

private:
    void setupApplicationProperties();
    void setupDarkTheme();
    void setupDirectories();
    bool setupDatabase();
    bool setupModules();

    std::unique_ptr<DatabaseManager> m_databaseManager;
    std::unique_ptr<ModuleManager> m_moduleManager;
    std::unique_ptr<QSettings> m_settings;
    
    static Application* s_instance;
    bool m_initialized;
};

// Global convenience function
inline Application* archiFlowApp() {
    return Application::instance();
}

#endif // APPLICATION_H
