#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sidebar.h"
#include "settingsdialog.h"
#include "core/application.h"
#include "core/modulemanager.h"
#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QDebug>
#include <QDir>
#include <QResource>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_centralWidget(nullptr)
    , m_sidebar(nullptr)
    , m_moduleStack(nullptr)
    , m_welcomeWidget(nullptr)
    , m_statusLabel(nullptr)
    , m_userLabel(nullptr)
    , m_timeLabel(nullptr)
    , m_mainToolBar(nullptr)
    , m_application(archiFlowApp())
{
    ui->setupUi(this);
    setupUi();
    connectSignals();    // Set window properties
    setWindowTitle("ArchiFlow - Architecture Office Management");
    setWindowIcon(QIcon(":/icons/ArchiFlow.svg"));
    resize(1400, 900);
    
    updateStatusBar();
    
    // Update time every minute
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateStatusBar);
    timer->start(60000); // 1 minute
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUi()
{
    // Create central widget with horizontal layout
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
      // Create sidebar
    m_sidebar = new Sidebar(this);    // Debug resource system
    qDebug() << "=== RESOURCE DEBUG ===";
    qDebug() << "QResource exists for :/icons/ArchiFlow.svg:" << QResource(":/icons/ArchiFlow.svg").isValid();
    qDebug() << "QResource exists for :/icons/a.png:" << QResource(":/icons/a.png").isValid();
    qDebug() << "QResource exists for :/ArchiFlow.svg:" << QResource(":/ArchiFlow.svg").isValid();
    qDebug() << "QResource exists for :/a.png:" << QResource(":/a.png").isValid();
    qDebug() << "Available resources in :/:" << QDir(":/").entryList();
    qDebug() << "Available resources in :/icons:" << QDir(":/icons").entryList();
    
    // Try creating a simple colored pixmap first to test the mechanism
    QPixmap testPixmap(64, 64);
    testPixmap.fill(Qt::red);
    qDebug() << "Test pixmap created - isNull:" << testPixmap.isNull() << "size:" << testPixmap.size();
    
    // Set the test pixmap first
    if (!testPixmap.isNull()) {
        m_sidebar->setLogo(testPixmap);
        qDebug() << "Test red pixmap set successfully";
    }
      // Try multiple resource paths for the logo
    QPixmap logo;
    QStringList logoPaths = {
        ":/icons/ArchiFlow.svg",
        ":/icons/ArchiFlow.png",
        ":/icons/a.png"
    };
    
    for (const QString &path : logoPaths) {
        logo = QPixmap(path);
        qDebug() << "Trying logo path:" << path << "- isNull:" << logo.isNull() << "size:" << logo.size();
        if (!logo.isNull()) {
            qDebug() << "Successfully loaded logo from:" << path;
            break;
        }
    }
    
    if (!logo.isNull()) {
        m_sidebar->setLogo(logo);
        m_sidebar->setLogoText(""); // Remove text since we have the image
        qDebug() << "Logo set successfully";
    } else {
        m_sidebar->setLogoText("ArchiFlow"); // Fallback text
        qDebug() << "Logo failed to load, using text fallback";
    }
    
    mainLayout->addWidget(m_sidebar);
    
    // Create module stack
    m_moduleStack = new QStackedWidget(this);
    mainLayout->addWidget(m_moduleStack);
    
    // Create welcome widget
    createWelcomeWidget();
    
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
}

void MainWindow::setupMenuBar()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    
    QAction *settingsAction = fileMenu->addAction("&Settings");
    settingsAction->setShortcut(QKeySequence::Preferences);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettingsTriggered);
    
    fileMenu->addSeparator();
    
    QAction *exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExitTriggered);
      // View menu
    // QMenu *viewMenu = menuBar()->addMenu("&View");
    // TODO: Add view options as modules are implemented
    
    // Tools menu
    // QMenu *toolsMenu = menuBar()->addMenu("&Tools");
    // TODO: Add tool actions as features are implemented
    
    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    
    QAction *aboutAction = helpMenu->addAction("&About ArchiFlow");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAboutTriggered);
    
    QAction *aboutQtAction = helpMenu->addAction("About &Qt");
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::setupToolBar()
{
    m_mainToolBar = addToolBar("Main");
    m_mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    // TODO: Add toolbar actions as modules are implemented
    // Example:
    // QAction *newProjectAction = m_mainToolBar->addAction(QIcon(":/icons/new_project.png"), "New Project");
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Ready", this);
    statusBar()->addWidget(m_statusLabel);
    
    statusBar()->addPermanentWidget(new QLabel("|"));
    
    m_userLabel = new QLabel("Guest User", this); // TODO: Replace with actual user
    statusBar()->addPermanentWidget(m_userLabel);
    
    statusBar()->addPermanentWidget(new QLabel("|"));
      m_timeLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_timeLabel);
}

void MainWindow::createWelcomeWidget()
{
    m_welcomeWidget = new QWidget(this);
    QVBoxLayout *welcomeLayout = new QVBoxLayout(m_welcomeWidget);
    welcomeLayout->setAlignment(Qt::AlignCenter);
    
    QLabel *welcomeLabel = new QLabel("Welcome to ArchiFlow", m_welcomeWidget);
    welcomeLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #2C3E50; margin: 20px;");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    
    QLabel *descLabel = new QLabel("Architecture Office Management System", m_welcomeWidget);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setStyleSheet("font-size: 16px; color: #7F8C8D; margin: 10px;");
    
    QLabel *instructionLabel = new QLabel("Select a module from the sidebar to get started.", m_welcomeWidget);
    instructionLabel->setAlignment(Qt::AlignCenter);
    instructionLabel->setStyleSheet("font-size: 14px; color: #95A5A6; margin: 5px;");
    
    welcomeLayout->addWidget(welcomeLabel);
    welcomeLayout->addWidget(descLabel);
    welcomeLayout->addWidget(instructionLabel);
    
    m_moduleStack->addWidget(m_welcomeWidget);
}

void MainWindow::connectSignals()
{
    connect(m_sidebar, &Sidebar::navigationItemClicked,
            this, &MainWindow::onNavigationChanged);
    connect(m_sidebar, &Sidebar::settingsClicked,
            this, &MainWindow::onSettingsRequested);
    
    if (m_application) {
        connect(m_application, &Application::applicationInitialized,
                this, &MainWindow::onApplicationInitialized);
    }
}

void MainWindow::addModule(BaseModule *module)
{
    if (!module) {
        return;
    }    qDebug() << "Adding module to main window:" << module->displayName();    // Add to sidebar navigation with appropriate icon
    QString icon = "📦"; // Default icon
    if (module->name() == "materials") {
        icon = "📦";
    } else if (module->name() == "contracts") {
        icon = "📜";
    } else if (module->name() == "projects") {
        icon = "🏗️";
    } else if (module->name() == "clients") {
        icon = "👥";
    }
    
    m_sidebar->addNavigationItem(module->displayName(), module->name(), icon);
    
    // Create and add module widget
    QWidget *moduleWidget = module->createWidget(this);
    if (moduleWidget) {
        m_moduleStack->addWidget(moduleWidget);
        
        // Store mapping for navigation
        m_sidebar->setProperty((module->name() + "_index").toUtf8(), 
                              m_moduleStack->count() - 1);
    }
}

void MainWindow::showModule(const QString &moduleName)
{
    m_sidebar->setCurrentItem(moduleName);
}

void MainWindow::onNavigationChanged(const QString &moduleName)
{
    if (moduleName.isEmpty() || moduleName == "dashboard") {
        // Show welcome/dashboard page
        m_moduleStack->setCurrentIndex(0);
        m_statusLabel->setText("Dashboard");
        return;
    }
    
    // Check if this is a registered module
    ModuleManager *moduleManager = m_application->moduleManager();
    if (moduleManager && moduleManager->module(moduleName)) {
        // Find the module widget index
        QVariant indexVar = m_sidebar->property((moduleName + "_index").toUtf8());
        if (indexVar.isValid()) {
            int index = indexVar.toInt();
            if (index < m_moduleStack->count()) {
                m_moduleStack->setCurrentIndex(index);
                m_statusLabel->setText(QString("Module: %1").arg(moduleName.toUpper()));
                return;
            }
        }
    }
    
    // Handle placeholder modules (not yet implemented)
    QMessageBox::information(this, "Module Not Available",
                           QString("The %1 module is not yet implemented.\n"
                                  "This feature will be available in a future update.")
                           .arg(moduleName.toUpper()));
    
    // Reset to dashboard
    m_sidebar->setCurrentItem("dashboard");
}

void MainWindow::onSettingsRequested()
{
    onSettingsTriggered();
}

void MainWindow::onApplicationInitialized()
{
    qDebug() << "Application initialized, setting up modules...";
    
    // Add dashboard first with icon
    m_sidebar->addNavigationItem("Dashboard", "dashboard", "📊");
    
    ModuleManager *moduleManager = m_application->moduleManager();
    if (moduleManager) {
        // Add all registered modules to the UI
        for (BaseModule *module : moduleManager->modules()) {
            addModule(module);
        }
          // Add navigation items for core architecture office modules
        QStringList registeredModules = moduleManager->moduleNames();
        
        // Core Business Modules
        if (!registeredModules.contains("projects")) {
            m_sidebar->addNavigationItem("Projects", "projects", "🏗️");
        }
        if (!registeredModules.contains("clients")) {
            m_sidebar->addNavigationItem("Clients", "clients", "👥");
        }
        if (!registeredModules.contains("employees")) {
            m_sidebar->addNavigationItem("Employees", "employees", "👨‍💼");
        }        // Materials & Resources (already implemented)
        if (!registeredModules.contains("materials")) {            m_sidebar->addNavigationItem("Materials", "materials", "📦");
        }
        // Suppliers functionality disabled
        // if (!registeredModules.contains("suppliers")) {
        //     m_sidebar->addNavigationItem("Suppliers", "suppliers", "🏭");
        // }        // Financial Management
        if (!registeredModules.contains("invoices")) {
            m_sidebar->addNavigationItem("Invoicing", "invoices", "💰");
        }
        
        // Settings (handled separately by sidebar)
    }
    
    // Set default selection to dashboard
    m_sidebar->setCurrentItem("dashboard");
    m_statusLabel->setText("Application ready");
}

void MainWindow::onAboutTriggered()
{
    QMessageBox::about(this, "About ArchiFlow",
        "<h3>ArchiFlow v1.0.0</h3>"
        "<p>Architecture Office Management System</p>"
        "<p>A comprehensive desktop application for managing "
        "architecture office operations including employees, "
        "clients, projects, contracts, and invoicing.</p>"
        "<p><b>Built with Qt " QT_VERSION_STR "</b></p>");
}

void MainWindow::onSettingsTriggered()
{
    SettingsDialog dialog(this);
    dialog.exec();
}

void MainWindow::onExitTriggered()
{
    close();
}

void MainWindow::updateStatusBar()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    m_timeLabel->setText(currentTime.toString("yyyy-MM-dd hh:mm"));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton ret = QMessageBox::question(this,
        "ArchiFlow",
        "Are you sure you want to exit ArchiFlow?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (m_application) {
            m_application->shutdown();
        }
        event->accept();
    } else {
        event->ignore();
    }
}
