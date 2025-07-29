#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Application;
class BaseModule;
class Sidebar;

/**
 * @brief The MainWindow class - Main application window
 * 
 * This is the primary window that hosts all feature modules and provides
 * the main navigation interface for the ArchiFlow application.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();    // Module integration
    void addModule(BaseModule *module);
    void showModule(const QString &moduleName);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onNavigationChanged(const QString &moduleName);
    void onSettingsRequested();
    void onApplicationInitialized();
    void onAboutTriggered();
    void onSettingsTriggered();
    void onExitTriggered();

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void connectSignals();
    void updateStatusBar();
    void createWelcomeWidget();

    Ui::MainWindow *ui;
    
    // Main layout components
    QWidget *m_centralWidget;
    Sidebar *m_sidebar;
    QStackedWidget *m_moduleStack;
    
    // Welcome page
    QWidget *m_welcomeWidget;
    
    // Status bar components
    QLabel *m_statusLabel;
    QLabel *m_userLabel;
    QLabel *m_timeLabel;
    
    // Toolbar
    QToolBar *m_mainToolBar;
    
    Application *m_application;
};

#endif // MAINWINDOW_H
