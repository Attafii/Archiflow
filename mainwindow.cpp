#include "mainwindow.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QTableWidgetItem>
#include <QtGui/QIcon>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QDialogButtonBox>
#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QDateTimeEdit>
#include <QtCore/QSettings>
#include <QtCore/QDateTime>
#include <QtCore/QTimeZone>
#include <QtCore/QUuid>
#include <QtCore/QUrlQuery>
#include <QtCore/QEventLoop>
#include <QtWidgets/QHeaderView>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), table(new QTableWidget), searchBar(new QLineEdit),
    searchFilter(new QComboBox), deleteButton(new QPushButton),
    searchButton(new QPushButton), addButton(new QPushButton),
    updateButton(new QPushButton), titleLabel(new QLabel("Employee Management")),
    networkManager(new QNetworkAccessManager(this))
{
    setupUI();
    setupDatabase();
    loadEmployees();

    // Initialize Google Calendar API
    oauth2 = new QOAuth2AuthorizationCodeFlow(this);
    calendarId = "primary"; // Use primary calendar by default

    // Apply the stylesheet
    QString styleSheet = R"(
        QMainWindow {
            background-color: #1a1a2e;
        }
        QLabel {
            color: #e94560;
            font-size: 24px;
            font-weight: bold;
        }
        QLineEdit {
            border: 1px solid #16213e;
            border-radius: 4px;
            padding: 5px;
            background-color: #0f3460;
            color: #e94560;
        }
        QPushButton {
            background-color: #e94560;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 5px 10px;
        }
        QPushButton:hover {
            background-color: #0f3460;
        }
        QTableWidget {
            border: 1px solid #16213e;
            border-radius: 4px;
            background-color: #0f3460;
            color: #e94560;
        }
QHeaderView::section {
            background-color: #e94560;
            color: white;
            padding: 5px;
        }
        QTabWidget::pane {
            border: 1px solid #16213e;
            background-color: #1a1a2e;
        }
        QTabBar::tab {
            background-color: #0f3460;
            color: white;
            padding: 8px 12px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background-color: #e94560;
        }
        QCalendarWidget {
            background-color: #0f3460;
            color: #e94560;
        }
        QCalendarWidget QAbstractItemView {
            selection-background-color: #e94560;
            selection-color: white;
        }
    )";
    setStyleSheet(styleSheet);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    // Create main layout with sidebar and content area
    QHBoxLayout *mainHLayout = new QHBoxLayout();
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainHLayout);

    // Create sidebar
    sidebar = new QWidget();
    sidebar->setFixedWidth(220);
    sidebar->setObjectName("sidebar");
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setSpacing(2);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);

    // App title/logo area
    QLabel *appLogo = new QLabel("Employees Manager");
    appLogo->setAlignment(Qt::AlignCenter);
    appLogo->setStyleSheet("font-size: 17px; font-weight: bold; color: white; background-color: #1e3a8a; padding: 20px;");

    // Create navigation buttons
    QPushButton *employeesNavBtn = new QPushButton("Employees");
    employeesNavBtn->setIcon(QIcon(":/icons/users.png"));
    employeesNavBtn->setIconSize(QSize(20, 20));
    employeesNavBtn->setCheckable(true);
    employeesNavBtn->setChecked(true);

    QPushButton *calendarNavBtn = new QPushButton("Calendar");
    calendarNavBtn->setIcon(QIcon(":/icons/calendar.png"));
    calendarNavBtn->setIconSize(QSize(20, 20));
    calendarNavBtn->setCheckable(true);

    // Add buttons to sidebar
    sidebarLayout->addWidget(appLogo);
    sidebarLayout->addSpacing(20);
    sidebarLayout->addWidget(employeesNavBtn);
    sidebarLayout->addWidget(calendarNavBtn);
    sidebarLayout->addStretch();

    // Create stacked widget for content
    contentStack = new QStackedWidget();
    contentStack->setObjectName("contentArea");

    // Create employee page
    employeeTab = new QWidget();
    setupEmployeePage();
    contentStack->addWidget(employeeTab);

    // Create calendar page
    calendarTab = new QWidget();
    setupCalendarTab();
    contentStack->addWidget(calendarTab);

    // Add sidebar and content to main layout
    mainHLayout->addWidget(sidebar);
    mainHLayout->addWidget(contentStack, 1);
    mainHLayout->setContentsMargins(0, 0, 0, 0);
    mainHLayout->setSpacing(0);

    // Connect navigation buttons
    connect(employeesNavBtn, &QPushButton::clicked, [=]() {
        contentStack->setCurrentIndex(0);
        employeesNavBtn->setChecked(true);
        calendarNavBtn->setChecked(false);
    });

    connect(calendarNavBtn, &QPushButton::clicked, [=]() {
        contentStack->setCurrentIndex(1);
        employeesNavBtn->setChecked(false);
        calendarNavBtn->setChecked(true);
    });

    setCentralWidget(centralWidget);

    // Apply modern navy-themed stylesheet
    QString styleSheet = R"(
        /* Main Application */
        QMainWindow, QDialog {
            background-color: #f8fafc;
            color: #334155;
            font-family: 'Segoe UI', Arial, sans-serif;
        }

        /* Sidebar Styling */
        #sidebar {
            background-color: #1e3a8a;
            min-width: 220px;
            max-width: 220px;
            padding: 0;
            margin: 0;
        }

        #sidebar QPushButton {
            background-color: transparent;
            color: #e2e8f0;
            border: none;
            border-radius: 0;
            text-align: left;
            padding: 12px 15px;
            font-size: 14px;
            font-weight: 500;
        }

        #sidebar QPushButton:hover {
            background-color: #2563eb;
        }

        #sidebar QPushButton:checked {
            background-color: #3b82f6;
            color: white;
            border-left: 4px solid #dbeafe;
        }

        /* Content Area */
        #contentArea {
            background-color: #f8fafc;
            border-left: 1px solid #e2e8f0;
            padding: 0;
        }

        /* Headers and Labels */
        QLabel {
            color: #334155;
        }

        QLabel[title="true"] {
            font-size: 24px;
            font-weight: bold;
            color: #1e3a8a;
            margin-bottom: 10px;
        }

        /* Input Controls */
        QLineEdit, QComboBox, QDateEdit, QDateTimeEdit {
            border: 1px solid #cbd5e1;
            border-radius: 4px;
            padding: 8px 12px;
            background-color: white;
            color: #334155;
            selection-background-color: #3b82f6;
        }

        QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QDateTimeEdit:focus {
            border: 1px solid #3b82f6;
        }

        QLineEdit:hover, QComboBox:hover, QDateEdit:hover, QDateTimeEdit:hover {
            border: 1px solid #94a3b8;
        }

        /* Buttons */
        QPushButton {
            background-color: #64748b;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-weight: 500;
        }

        QPushButton:hover {
            background-color: #475569;
        }

        QPushButton:pressed {
            background-color: #334155;
        }

        QPushButton[class="primary"] {
            background-color: #1e40af;
        }

        QPushButton[class="primary"]:hover {
            background-color: #1e3a8a;
        }

        QPushButton[class="danger"] {
            background-color: #dc2626;
        }

        QPushButton[class="danger"]:hover {
            background-color: #b91c1c;
        }

        /* Tables */
        QTableWidget {
            background-color: white;
            alternate-background-color: #f1f5f9;
            border: 1px solid #e2e8f0;
            border-radius: 4px;
            gridline-color: #e2e8f0;
        }

        QTableWidget::item {
            padding: 6px;
            border-bottom: 1px solid #e2e8f0;
        }

        QTableWidget::item:selected {
            background-color: #dbeafe;
            color: #1e3a8a;
        }

        QHeaderView::section {
            background-color: #1e3a8a;
            color: white;
            padding: 8px;
            font-weight: bold;
            border: none;
            border-right: 1px solid #2563eb;
        }
/* Calendar Widget */
        QCalendarWidget {
            background-color: white;
            border: 1px solid #e2e8f0;
        }

        QCalendarWidget QToolButton {
            background-color: #1e3a8a;
            color: white;
            border-radius: 4px;
            padding: 4px;
        }

        QCalendarWidget QMenu {
            background-color: white;
            border: 1px solid #e2e8f0;
        }

        QCalendarWidget QSpinBox {
            background-color: white;
            border: 1px solid #cbd5e1;
            border-radius: 4px;
            padding: 2px;
        }

        QCalendarWidget QAbstractItemView:enabled {
            background-color: white;
            color: #334155;
            selection-background-color: #dbeafe;
            selection-color: #1e3a8a;
        }

        QCalendarWidget QAbstractItemView:disabled {
            color: #cbd5e1;
        }

        /* Scrollbars */
        QScrollBar:vertical {
            border: none;
            background-color: #f1f5f9;
            width: 10px;
            margin: 0;
        }

        QScrollBar::handle:vertical {
            background-color: #94a3b8;
            border-radius: 5px;
            min-height: 20px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: #64748b;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
            width: 0;
        }

        QScrollBar:horizontal {
            border: none;
            background-color: #f1f5f9;
            height: 10px;
            margin: 0;
        }

        QScrollBar::handle:horizontal {
            background-color: #94a3b8;
            border-radius: 5px;
            min-width: 20px;
        }

        QScrollBar::handle:horizontal:hover {
            background-color: #64748b;
        }

        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            height: 0;
            width: 0;
        }

        /* Dialog boxes */
        QDialog {
            background-color: #f8fafc;
        }

        QDialog QDialogButtonBox QPushButton {
            min-width: 80px;
        }

        QDialog QDialogButtonBox QPushButton[text="OK"],
        QDialog QDialogButtonBox QPushButton[text="&OK"] {
            background-color: #1e40af;
        }

        QDialog QDialogButtonBox QPushButton[text="Cancel"],
        QDialog QDialogButtonBox QPushButton[text="&Cancel"] {
            background-color: #64748b;
        }
    )";

    setStyleSheet(styleSheet);
}


void MainWindow::setupEmployeePage() {
    QVBoxLayout *employeeLayout = new QVBoxLayout(employeeTab);
    employeeLayout->setContentsMargins(20, 20, 20, 20);

    // Header with title and search
    QHBoxLayout *headerLayout = new QHBoxLayout();

    titleLabel = new QLabel("Employees");
    titleLabel->setProperty("title", true);

    QWidget *searchWidget = new QWidget();
    QHBoxLayout *searchLayout = new QHBoxLayout(searchWidget);
    searchLayout->setContentsMargins(0, 0, 0, 0);

    searchBar = new QLineEdit();
    searchBar->setPlaceholderText("Search employee...");
    searchBar->setMinimumWidth(250);

    searchFilter = new QComboBox();
    searchFilter->addItems({"CIN", "Nom", "Mail", "Poste", "Prenom", "Date d'embauche", "Role", "Presence actuelle", "Statut", "Telephone"});

    searchButton = new QPushButton();
    searchButton->setIcon(QIcon(":/icons/search.png"));
    searchButton->setIconSize(QSize(16, 16));
    searchButton->setFixedSize(36, 36);

    searchLayout->addWidget(searchBar);
    searchLayout->addWidget(searchFilter);
    searchLayout->addWidget(searchButton);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(searchWidget);

    // Action buttons
    QHBoxLayout *actionLayout = new QHBoxLayout();

    addButton = new QPushButton("Add Employee");
    addButton->setIcon(QIcon(":/icons/add.png"));
    addButton->setProperty("class", "primary");

    updateButton = new QPushButton("Edit");
    updateButton->setIcon(QIcon(":/icons/update.png"));

    deleteButton = new QPushButton("Delete");
    deleteButton->setIcon(QIcon(":/icons/delete.png"));
    deleteButton->setProperty("class", "danger");

    QPushButton *resetButton = new QPushButton("Reset Database");
    resetButton->setIcon(QIcon(":/icons/reset.png"));

    actionLayout->addWidget(addButton);
    actionLayout->addWidget(updateButton);
    actionLayout->addWidget(deleteButton);
    actionLayout->addStretch();
    actionLayout->addWidget(resetButton);

    // Table
    table = new QTableWidget();
    table->setColumnCount(10);
    table->setHorizontalHeaderLabels({"CIN", "Nom", "Mail", "Poste", "Prenom", "Date d'embauche", "Role", "Presence actuelle", "Statut", "Telephone"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);

    // Add all components to the layout
    employeeLayout->addLayout(headerLayout);
    employeeLayout->addSpacing(20);
    employeeLayout->addLayout(actionLayout);
    employeeLayout->addSpacing(10);
    employeeLayout->addWidget(table);

    // Connect signals
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::searchEmployee);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelected);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addEmployee);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateEmployee);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetDatabase);
}



void MainWindow::setupCalendarTab() {
    QVBoxLayout *mainLayout = new QVBoxLayout(calendarTab);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Calendar title
    calendarTitle = new QLabel("Employee Calendar");
    calendarTitle->setProperty("title", true);

    // Top controls
    QHBoxLayout *controlsLayout = new QHBoxLayout();

    // Employee filter
    QLabel *filterLabel = new QLabel("Filter by employee:");
    employeeFilter = new QComboBox();
    employeeFilter->addItem("All Employees");

    // Event type filter
    QLabel *eventTypeLabel = new QLabel("Event type:");
    QComboBox *eventTypeFilter = new QComboBox();
    eventTypeFilter->addItems({"All Events", "Vacations", "Meetings", "Working Hours", "Training"});

    // Add event button
    addEventButton = new QPushButton("Add Event");
    addEventButton->setIcon(QIcon(":/icons/add.png"));
    addEventButton->setProperty("class", "primary");

    // Delete event button
    deleteEventButton = new QPushButton("Delete Event");
    deleteEventButton->setIcon(QIcon(":/icons/delete.png"));
    deleteEventButton->setProperty("class", "danger");

    // Sync with Google button
    QPushButton *syncButton = new QPushButton("Sync with Google");
    syncButton->setIcon(QIcon(":/icons/sync.png"));

    controlsLayout->addWidget(filterLabel);
    controlsLayout->addWidget(employeeFilter);
    controlsLayout->addWidget(eventTypeLabel);
    controlsLayout->addWidget(eventTypeFilter);
    controlsLayout->addStretch();
    controlsLayout->addWidget(addEventButton);
    controlsLayout->addWidget(deleteEventButton);
    controlsLayout->addWidget(syncButton);

    // Create a container for calendar and events
    QWidget *calendarContainer = new QWidget();
    QHBoxLayout *calendarLayout = new QHBoxLayout(calendarContainer);
    calendarLayout->setContentsMargins(0, 0, 0, 0);

    // Calendar widget
    calendarWidget = new QCalendarWidget();
    calendarWidget->setGridVisible(true);
    calendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calendarWidget->setMinimumWidth(400);

    // Events panel
    QWidget *eventsPanel = new QWidget();
    QVBoxLayout *eventsPanelLayout = new QVBoxLayout(eventsPanel);

    QLabel *eventsTitle = new QLabel("Events for Selected Date");
    eventsTitle->setStyleSheet("font-weight: bold; font-size: 16px; color: #1e3a8a;");

    eventsTable = new QTableWidget();
    eventsTable->setObjectName("eventsTable");
    eventsTable->setColumnCount(5);
    eventsTable->setHorizontalHeaderLabels({"Employee", "Event Type", "Start Time", "End Time", "Description"});
    eventsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    eventsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    eventsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    eventsTable->verticalHeader()->setVisible(false);
    eventsTable->setAlternatingRowColors(true);

    eventsPanelLayout->addWidget(eventsTitle);
    eventsPanelLayout->addWidget(eventsTable);

    // Add calendar and events panel to the container
    calendarLayout->addWidget(calendarWidget, 1);
    calendarLayout->addWidget(eventsPanel, 1);

    // Add widgets to main layout
    mainLayout->addWidget(calendarTitle);
    mainLayout->addLayout(controlsLayout);
    mainLayout->addWidget(calendarContainer, 1);

    // Connect signals
    connect(calendarWidget, &QCalendarWidget::selectionChanged, this, &MainWindow::updateCalendarView);
    connect(employeeFilter, &QComboBox::currentTextChanged, this, &MainWindow::updateCalendarView);
    connect(eventTypeFilter, &QComboBox::currentTextChanged, this, &MainWindow::updateCalendarView);
    connect(addEventButton, &QPushButton::clicked, this, &MainWindow::addCalendarEvent);
    connect(deleteEventButton, &QPushButton::clicked, this, &MainWindow::deleteCalendarEvent);
    connect(syncButton, &QPushButton::clicked, this, &MainWindow::authenticateWithGoogle);

    // Update employee filter
    updateEmployeeFilter();
}

void MainWindow::setupDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("employees.db");
    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", db.lastError().text());
        return;
    }
    QSqlQuery query;
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS employees (
            cin TEXT PRIMARY KEY,
            nom TEXT,
            mail TEXT,
            poste TEXT,
            prenom TEXT,
            date_embauche TEXT,
            role TEXT,
            presence_actuelle TEXT,
            statut TEXT,
            telephone TEXT
        )
    )");

    // Create events table for calendar
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS events (
            id TEXT PRIMARY KEY,
            employee_cin TEXT,
            event_type TEXT,
            start_time TEXT,
            end_time TEXT,
            description TEXT,
            google_event_id TEXT,
            FOREIGN KEY (employee_cin) REFERENCES employees (cin)
        )
    )");
}

void MainWindow::loadEmployees() {
    table->setRowCount(0);
    QSqlQuery query("SELECT * FROM employees");
    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        // Map database columns to table columns with CIN as the first column
        table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString())); // cin
        table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString())); // nom
        table->setItem(row, 2, new QTableWidgetItem(query.value(2).toString())); // mail
        table->setItem(row, 3, new QTableWidgetItem(query.value(3).toString())); // poste
        table->setItem(row, 4, new QTableWidgetItem(query.value(4).toString())); // prenom
        table->setItem(row, 5, new QTableWidgetItem(query.value(5).toString())); // date_embauche
        table->setItem(row, 6, new QTableWidgetItem(query.value(6).toString())); // role
        table->setItem(row, 7, new QTableWidgetItem(query.value(7).toString())); // presence_actuelle
        table->setItem(row, 8, new QTableWidgetItem(query.value(8).toString())); // statut
        table->setItem(row, 9, new QTableWidgetItem(query.value(9).toString())); // telephone
        row++;
    }

    // Update employee filter in calendar tab
    updateEmployeeFilter();
}

void MainWindow::searchEmployee() {
    QString keyword = searchBar->text();
    QString filter = searchFilter->currentText();

    // Map display column names to database column names
    QString columnName;
    if (filter == "CIN") columnName = "cin";
    else if (filter == "Nom") columnName = "nom";
    else if (filter == "Mail") columnName = "mail";
    else if (filter == "Poste") columnName = "poste";
    else if (filter == "Prenom") columnName = "prenom";
    else if (filter == "Date d'embauche") columnName = "date_embauche";
    else if (filter == "Role") columnName = "role";
    else if (filter == "Presence actuelle") columnName = "presence_actuelle";
    else if (filter == "Statut") columnName = "statut";
    else if (filter == "Telephone") columnName = "telephone";

    QSqlQuery query;
    query.prepare(QString("SELECT * FROM employees WHERE %1 LIKE ?").arg(columnName));
    query.addBindValue("%" + keyword + "%");
    query.exec();
    table->setRowCount(0);
    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        // Map database columns to table columns with CIN as the first column
        table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString())); // cin
        table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString())); // nom
        table->setItem(row, 2, new QTableWidgetItem(query.value(2).toString())); // mail
        table->setItem(row, 3, new QTableWidgetItem(query.value(3).toString())); // poste
        table->setItem(row, 4, new QTableWidgetItem(query.value(4).toString())); // prenom
        table->setItem(row, 5, new QTableWidgetItem(query.value(5).toString())); // date_embauche
        table->setItem(row, 6, new QTableWidgetItem(query.value(6).toString())); // role
        table->setItem(row, 7, new QTableWidgetItem(query.value(7).toString())); // presence_actuelle
        table->setItem(row, 8, new QTableWidgetItem(query.value(8).toString())); // statut
        table->setItem(row, 9, new QTableWidgetItem(query.value(9).toString())); // telephone
        row++;
    }
}

void MainWindow::deleteSelected() {
    QList<QTableWidgetItem *> selected = table->selectedItems();
    if (selected.isEmpty()) return;
    int row = table->row(selected.first());
    QString cin = table->item(row, 0)->text(); // CIN is now in the first column
    QSqlQuery query;
    query.prepare("DELETE FROM employees WHERE cin = ?");
    query.addBindValue(cin);
    if (!query.exec()) {
        QMessageBox::warning(this, "Delete Failed", query.lastError().text());
    }
    loadEmployees();
}

void MainWindow::addEmployee() {
    AddEmployeeDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString nom = dialog.getNom();
        QString mail = dialog.getMail();
        QString poste = dialog.getPoste();
        QString prenom = dialog.getPrenom();
        QString dateEmbauche = dialog.getDateEmbauche();
        QString role = dialog.getRole();
        QString presenceActuelle = dialog.getPresenceActuelle();
        QString statut = dialog.getStatut();
        QString telephone = dialog.getTelephone();
        QString cin = dialog.getCin();
        QSqlQuery query;
        query.prepare("INSERT INTO employees (cin, nom, mail, poste, prenom, date_embauche, role, presence_actuelle, statut, telephone) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(cin);
        query.addBindValue(nom);
        query.addBindValue(mail);
        query.addBindValue(poste);
        query.addBindValue(prenom);
        query.addBindValue(dateEmbauche);
        query.addBindValue(role);
        query.addBindValue(presenceActuelle);
        query.addBindValue(statut);
        query.addBindValue(telephone);
        if (!query.exec()) {
            QMessageBox::warning(this, "Add Failed", query.lastError().text());
        }
        loadEmployees();
    }
}
void MainWindow::updateEmployee() {
    QList<QTableWidgetItem *> selected = table->selectedItems();
    if (selected.isEmpty()) return;
    int row = table->row(selected.first());
    QString cin = table->item(row, 0)->text();      // CIN is now in the first column
    QString nom = table->item(row, 1)->text();      // nom is now in the second column
    QString mail = table->item(row, 2)->text();     // mail is now in the third column
    QString poste = table->item(row, 3)->text();    // poste is now in the fourth column
    QString prenom = table->item(row, 4)->text();   // prenom is now in the fifth column
    QString dateEmbauche = table->item(row, 5)->text(); // date_embauche is now in the sixth column
    QString role = table->item(row, 6)->text();     // role is now in the seventh column
    QString presenceActuelle = table->item(row, 7)->text(); // presence_actuelle is now in the eighth column
    QString statut = table->item(row, 8)->text();   // statut is now in the ninth column
    QString telephone = table->item(row, 9)->text(); // telephone is now in the tenth column

    UpdateEmployeeDialog dialog(this, cin, nom, mail, poste, prenom, dateEmbauche, role, presenceActuelle, statut, telephone);
    if (dialog.exec() == QDialog::Accepted) {
        QString newNom = dialog.getNom();
        QString newMail = dialog.getMail();
        QString newPoste = dialog.getPoste();
        QString newPrenom = dialog.getPrenom();
        QString newDateEmbauche = dialog.getDateEmbauche();
        QString newRole = dialog.getRole();
        QString newPresenceActuelle = dialog.getPresenceActuelle();
        QString newStatut = dialog.getStatut();
        QString newTelephone = dialog.getTelephone();
        QSqlQuery query;
        query.prepare("UPDATE employees SET nom = ?, mail = ?, poste = ?, prenom = ?, date_embauche = ?, role = ?, presence_actuelle = ?, statut = ?, telephone = ? WHERE cin = ?");
        query.addBindValue(newNom);
        query.addBindValue(newMail);
        query.addBindValue(newPoste);
        query.addBindValue(newPrenom);
        query.addBindValue(newDateEmbauche);
        query.addBindValue(newRole);
        query.addBindValue(newPresenceActuelle);
        query.addBindValue(newStatut);
        query.addBindValue(newTelephone);
        query.addBindValue(cin);
        if (!query.exec()) {
            QMessageBox::warning(this, "Update Failed", query.lastError().text());
        }
        loadEmployees();
    }
}

void MainWindow::resetDatabase() {
    // Close any existing connections
    if (db.isOpen()) {
        db.close();
    }

    // Remove the database file
    QFile file("employees.db");
    if (file.exists()) {
        if (file.remove()) {
            qDebug() << "Database file removed successfully";
        } else {
            qDebug() << "Failed to remove database file:" << file.errorString();
        }
    } else {
        qDebug() << "Database file does not exist";
    }

    // Reopen and recreate the database
    setupDatabase();
    loadEmployees();

    QMessageBox::information(this, "Database Reset", "The database has been reset successfully.");
}

// Calendar functionality
void MainWindow::updateEmployeeFilter() {
    // Store current selection
    QString currentSelection = employeeFilter->currentText();

    // Clear and repopulate the filter
    employeeFilter->clear();
    employeeFilter->addItem("All Employees");

    // Query all employees
    QSqlQuery query("SELECT cin, nom, prenom FROM employees ORDER BY nom, prenom");
    while (query.next()) {
        QString cin = query.value(0).toString();
        QString nom = query.value(1).toString();
        QString prenom = query.value(2).toString();
        QString displayName = nom + " " + prenom;

        // Store the CIN as user data
        employeeFilter->addItem(displayName, cin);
    }

    // Restore previous selection if it exists
    int index = employeeFilter->findText(currentSelection);
    if (index >= 0) {
        employeeFilter->setCurrentIndex(index);
    }
}

void MainWindow::updateCalendarView() {
    // Get selected date
    QDate selectedDate = calendarWidget->selectedDate();

    // Get selected employee
    QString employeeCin;
    if (employeeFilter->currentIndex() > 0) {
        employeeCin = employeeFilter->currentData().toString();
    }

    // Clear events table
    eventsTable->setRowCount(0);

    // Query events for the selected date
    QSqlQuery query;
    if (employeeCin.isEmpty()) {
        query.prepare("SELECT e.id, e.employee_cin, e.event_type, e.start_time, e.end_time, e.description, "
                      "emp.nom, emp.prenom FROM events e "
                      "LEFT JOIN employees emp ON e.employee_cin = emp.cin "
                      "WHERE date(e.start_time) = ? OR date(e.end_time) = ? "
                      "ORDER BY e.start_time");
        query.addBindValue(selectedDate.toString(Qt::ISODate));
        query.addBindValue(selectedDate.toString(Qt::ISODate));
    } else {
        query.prepare("SELECT e.id, e.employee_cin, e.event_type, e.start_time, e.end_time, e.description, "
                      "emp.nom, emp.prenom FROM events e "
                      "LEFT JOIN employees emp ON e.employee_cin = emp.cin "
                      "WHERE e.employee_cin = ? AND (date(e.start_time) = ? OR date(e.end_time) = ?) "
                      "ORDER BY e.start_time");
        query.addBindValue(employeeCin);
        query.addBindValue(selectedDate.toString(Qt::ISODate));
        query.addBindValue(selectedDate.toString(Qt::ISODate));
    }

    if (!query.exec()) {
        qDebug() << "Error querying events:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        QString employeeName = query.value(6).toString() + " " + query.value(7).toString();
        QString eventType = query.value(2).toString();
        QDateTime startTime = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
        QDateTime endTime = QDateTime::fromString(query.value(4).toString(), Qt::ISODate);
        QString description = query.value(5).toString();

        eventsTable->insertRow(row);
        eventsTable->setItem(row, 0, new QTableWidgetItem(employeeName));
        eventsTable->setItem(row, 1, new QTableWidgetItem(eventType));
        eventsTable->setItem(row, 2, new QTableWidgetItem(startTime.toString("yyyy-MM-dd hh:mm AP")));
        eventsTable->setItem(row, 3, new QTableWidgetItem(endTime.toString("yyyy-MM-dd hh:mm AP")));
        eventsTable->setItem(row, 4, new QTableWidgetItem(description));

        // Store event ID as hidden data
        eventsTable->item(row, 0)->setData(Qt::UserRole, query.value(0).toString());

        row++;
    }

    // Resize columns to content
    eventsTable->resizeColumnsToContents();
}

    void MainWindow::addCalendarEvent() {
    // Get list of employees for the dialog
    QStringList employees;
    QMap<QString, QString> employeeCinMap;

    QSqlQuery query("SELECT cin, nom, prenom FROM employees ORDER BY nom, prenom");
    while (query.next()) {
        QString cin = query.value(0).toString();
        QString nom = query.value(1).toString();
        QString prenom = query.value(2).toString();
        QString displayName = nom + " " + prenom;

        employees << displayName;
        employeeCinMap[displayName] = cin;
    }

    if (employees.isEmpty()) {
        QMessageBox::warning(this, "Add Event", "You need to add employees first before creating events.");
        return;
    }

    // Create and show the add event dialog
    AddEventDialog dialog(this, employees);
    if (dialog.exec() == QDialog::Accepted) {
        QString employee = dialog.getEmployee();
        QString eventType = dialog.getEventType();
        QDateTime startTime = dialog.getStartTime();
        QDateTime endTime = dialog.getEndTime();
        QString description = dialog.getDescription();

        // Get employee CIN from the map
        QString employeeCin = employeeCinMap[employee];

        // Generate a unique ID for the event
        QString eventId = QUuid::createUuid().toString(QUuid::WithoutBraces);

        // Insert the event into the database
        QSqlQuery insertQuery;
        insertQuery.prepare("INSERT INTO events (id, employee_cin, event_type, start_time, end_time, description) "
                            "VALUES (?, ?, ?, ?, ?, ?)");
        insertQuery.addBindValue(eventId);
        insertQuery.addBindValue(employeeCin);
        insertQuery.addBindValue(eventType);
        insertQuery.addBindValue(startTime.toString(Qt::ISODate));
        insertQuery.addBindValue(endTime.toString(Qt::ISODate));
        insertQuery.addBindValue(description);

        if (!insertQuery.exec()) {
            QMessageBox::warning(this, "Add Event Failed", insertQuery.lastError().text());
            return;
        }

        // Update the calendar view
        updateCalendarView();

        QMessageBox::information(this, "Add Event", "Event added successfully.");
    }
}

void MainWindow::deleteCalendarEvent() {
    // Check if an event is selected
    QList<QTableWidgetItem *> selected = eventsTable->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Delete Event", "Please select an event to delete.");
        return;
    }

    // Get the event ID from the hidden data
    int row = eventsTable->row(selected.first());
    QString eventId = eventsTable->item(row, 0)->data(Qt::UserRole).toString();

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete Event",
                                                              "Are you sure you want to delete this event?",
                                                              QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) {
        return;
    }

    // Delete the event from the database
    QSqlQuery deleteQuery;
    deleteQuery.prepare("DELETE FROM events WHERE id = ?");
    deleteQuery.addBindValue(eventId);

    if (!deleteQuery.exec()) {
        QMessageBox::warning(this, "Delete Event Failed", deleteQuery.lastError().text());
        return;
    }

    // Update the calendar view
    updateCalendarView();

    QMessageBox::information(this, "Delete Event", "Event deleted successfully.");
}

void MainWindow::authenticateWithGoogle() {
    // Set up OAuth2 flow
    oauth2->setScope("https://www.googleapis.com/auth/calendar");

    // Load OAuth credentials from environment variables
    QString clientId = qgetenv("GOOGLE_CLIENT_ID");
    QString clientSecret = qgetenv("GOOGLE_CLIENT_SECRET");
    
    if (clientId.isEmpty() || clientSecret.isEmpty()) {
        QMessageBox::warning(this, "Configuration Error", 
                           "Google OAuth credentials not found. Please set GOOGLE_CLIENT_ID and GOOGLE_CLIENT_SECRET environment variables.");
        return;
    }
    
    oauth2->setClientIdentifier(clientId);
    oauth2->setClientIdentifierSharedKey(clientSecret);

    // Set up authorization URL and token URL
    oauth2->setAuthorizationUrl(QUrl("https://accounts.google.com/o/oauth2/auth"));
    oauth2->setAccessTokenUrl(QUrl("https://oauth2.googleapis.com/token"));

    // Connect signals
    connect(oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &QDesktopServices::openUrl);

    connect(oauth2, &QOAuth2AuthorizationCodeFlow::granted, [=]() {
        QMessageBox::information(this, "Authentication", "Successfully authenticated with Google Calendar!");
        fetchCalendarEvents();
    });

    // Start the authentication flow
    oauth2->grant();
}

void MainWindow::fetchCalendarEvents() {
    // Create request to Google Calendar API
    QUrl url("https://www.googleapis.com/calendar/v3/calendars/primary/events");
    QUrlQuery query;
    query.addQueryItem("maxResults", "100");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Send the request with OAuth2 token
    request.setRawHeader("Authorization", "Bearer " + oauth2->token().toUtf8());
    QNetworkReply *reply = networkManager->get(request);

    // Handle the response
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Parse JSON response
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject obj = doc.object();
            QJsonArray events = obj["items"].toArray();

            // Process the events
            processCalendarEvents(events);
        } else {
            QMessageBox::warning(this, "Google Calendar Error",
                                 "Failed to fetch events: " + reply->errorString());
        }

        reply->deleteLater();
    });
}

void MainWindow::processCalendarEvents(const QJsonArray &events) {
    // First, get a map of employee names to CINs
    QMap<QString, QString> employeeNameToCin;
    QSqlQuery employeeQuery("SELECT cin, nom, prenom FROM employees");
    while (employeeQuery.next()) {
        QString cin = employeeQuery.value(0).toString();
        QString fullName = employeeQuery.value(1).toString() + " " + employeeQuery.value(2).toString();
        employeeNameToCin[fullName] = cin;
    }

    // Process each event from Google Calendar
    int importedCount = 0;

    for (const QJsonValue &eventValue : events) {
        QJsonObject event = eventValue.toObject();

        // Skip deleted events
        if (event.contains("status") && event["status"].toString() == "cancelled") {
            continue;
        }

        QString googleEventId = event["id"].toString();
        QString summary = event["summary"].toString();

        // Try to extract employee name and event type from summary
        // Format expected: "Employee Name - Event Type: Description"
        QString employeeName;
        QString eventType = "Meeting"; // Default event type
        QString description = summary;

        int dashPos = summary.indexOf(" - ");
        if (dashPos > 0) {
            employeeName = summary.left(dashPos);

            int colonPos = summary.indexOf(": ", dashPos);
            if (colonPos > 0) {
                eventType = summary.mid(dashPos + 3, colonPos - dashPos - 3);
                description = summary.mid(colonPos + 2);
            } else {
                description = summary.mid(dashPos + 3);
            }
        }

        // Get start and end times
        QDateTime startTime;
        QDateTime endTime;

        if (event["start"].toObject().contains("dateTime")) {
            // This is a time-specific event
            QString startTimeStr = event["start"].toObject()["dateTime"].toString();
            QString endTimeStr = event["end"].toObject()["dateTime"].toString();

            startTime = QDateTime::fromString(startTimeStr, Qt::ISODate);
            endTime = QDateTime::fromString(endTimeStr, Qt::ISODate);
        } else {
            // This is an all-day event
            QString startDateStr = event["start"].toObject()["date"].toString();
            QString endDateStr = event["end"].toObject()["date"].toString();

            QDate startDate = QDate::fromString(startDateStr, Qt::ISODate);
            QDate endDate = QDate::fromString(endDateStr, Qt::ISODate);

            startTime = QDateTime(startDate, QTime(0, 0));
            endTime = QDateTime(endDate.addDays(-1), QTime(23, 59));
        }

        // Find the employee CIN
        QString employeeCin;
        if (employeeNameToCin.contains(employeeName)) {
            employeeCin = employeeNameToCin[employeeName];
        } else {
            // If we can't find the employee, use the first one in the database
            QSqlQuery firstEmployeeQuery("SELECT cin FROM employees LIMIT 1");
            if (firstEmployeeQuery.next()) {
                employeeCin = firstEmployeeQuery.value(0).toString();
            } else {
                // No employees in the database, skip this event
                continue;
            }
        }

        // Check if this Google event already exists in our database
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT id FROM events WHERE google_event_id = ?");
        checkQuery.addBindValue(googleEventId);

        if (checkQuery.exec() && checkQuery.next()) {
            // Event already exists, update it
            QString localEventId = checkQuery.value(0).toString();

            QSqlQuery updateQuery;
            updateQuery.prepare("UPDATE events SET employee_cin = ?, event_type = ?, "
                                "start_time = ?, end_time = ?, description = ? "
                                "WHERE id = ?");
            updateQuery.addBindValue(employeeCin);
            updateQuery.addBindValue(eventType);
            updateQuery.addBindValue(startTime.toString(Qt::ISODate));
            updateQuery.addBindValue(endTime.toString(Qt::ISODate));
            updateQuery.addBindValue(description);
            updateQuery.addBindValue(localEventId);

            if (updateQuery.exec()) {
                importedCount++;
            } else {
                qDebug() << "Failed to update event:" << updateQuery.lastError().text();
            }
        } else {
            // New event, insert it
            QString eventId = QUuid::createUuid().toString(QUuid::WithoutBraces);

            QSqlQuery insertQuery;
            insertQuery.prepare("INSERT INTO events (id, employee_cin, event_type, start_time, end_time, "
                                "description, google_event_id) VALUES (?, ?, ?, ?, ?, ?, ?)");
            insertQuery.addBindValue(eventId);
            insertQuery.addBindValue(employeeCin);
            insertQuery.addBindValue(eventType);
            insertQuery.addBindValue(startTime.toString(Qt::ISODate));
            insertQuery.addBindValue(endTime.toString(Qt::ISODate));
            insertQuery.addBindValue(description);
            insertQuery.addBindValue(googleEventId);

            if (insertQuery.exec()) {
                importedCount++;
            } else {
                qDebug() << "Failed to insert event:" << insertQuery.lastError().text();
            }
        }
    }

    // Update the calendar view
    updateCalendarView();

    QMessageBox::information(this, "Import Complete",
                             QString("Successfully imported/updated %1 events from Google Calendar.").arg(importedCount));
}
void MainWindow::syncEventsWithGoogle() {
    // First authenticate with Google if not already authenticated
    if (!oauth2->token().isEmpty()) {
        // Already authenticated, proceed with sync
        QMessageBox::information(this, "Sync", "Syncing events with Google Calendar...");

        // Get all local events that don't have a Google event ID
        QSqlQuery query("SELECT id, employee_cin, event_type, start_time, end_time, description "
                        "FROM events WHERE google_event_id IS NULL");

        int syncedCount = 0;

        while (query.next()) {
            QString eventId = query.value(0).toString();
            QString employeeCin = query.value(1).toString();
            QString eventType = query.value(2).toString();
            QDateTime startTime = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
            QDateTime endTime = QDateTime::fromString(query.value(4).toString(), Qt::ISODate);
            QString description = query.value(5).toString();

            // Get employee name
            QSqlQuery employeeQuery;
            employeeQuery.prepare("SELECT nom, prenom FROM employees WHERE cin = ?");
            employeeQuery.addBindValue(employeeCin);

            QString employeeName;
            if (employeeQuery.exec() && employeeQuery.next()) {
                employeeName = employeeQuery.value(0).toString() + " " + employeeQuery.value(1).toString();
            } else {
                employeeName = "Unknown Employee";
            }

            // Create event summary in the format "Employee Name - Event Type: Description"
            QString summary = employeeName + " - " + eventType;
            if (!description.isEmpty()) {
                summary += ": " + description;
            }

            // Create the event in Google Calendar
            QJsonObject event;
            event["summary"] = summary;

            QJsonObject start;
            start["dateTime"] = startTime.toString(Qt::ISODate);
            start["timeZone"] = QString(QTimeZone::systemTimeZoneId());

            QJsonObject end;
            end["dateTime"] = endTime.toString(Qt::ISODate);
            end["timeZone"] = QString(QTimeZone::systemTimeZoneId());

            event["start"] = start;
            event["end"] = end;

            // Create the request
            QUrl url("https://www.googleapis.com/calendar/v3/calendars/primary/events");
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

            // Send the request
            request.setRawHeader("Authorization", "Bearer " + oauth2->token().toUtf8());
            QNetworkReply *reply = networkManager->post(request, QJsonDocument(event).toJson());

            // Handle the response
            QEventLoop loop;
            connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            loop.exec();

            if (reply->error() == QNetworkReply::NoError) {
                // Parse the response to get the Google event ID
                QByteArray data = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(data);
                QJsonObject obj = doc.object();
                QString googleEventId = obj["id"].toString();

                // Update the local event with the Google event ID
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE events SET google_event_id = ? WHERE id = ?");
                updateQuery.addBindValue(googleEventId);
                updateQuery.addBindValue(eventId);

                if (updateQuery.exec()) {
                    syncedCount++;
                } else {
                    qDebug() << "Failed to update event with Google ID:" << updateQuery.lastError().text();
                }
            } else {
                qDebug() << "Failed to create event in Google Calendar:" << reply->errorString();
            }

            reply->deleteLater();
        }

        QMessageBox::information(this, "Sync Complete",
                                 QString("Successfully synced %1 events to Google Calendar.").arg(syncedCount));
    } else {
        // Not authenticated, start authentication flow
        authenticateWithGoogle();
    }
}

// Dialog implementations
AddEmployeeDialog::AddEmployeeDialog(QWidget *parent)
    : QDialog(parent), nomEdit(new QLineEdit), mailEdit(new QLineEdit), posteEdit(new QLineEdit),
    prenomEdit(new QLineEdit), dateEmbaucheEdit(new QLineEdit), roleEdit(new QLineEdit),
    presenceActuelleEdit(new QLineEdit), statutEdit(new QLineEdit), telephoneEdit(new QLineEdit),
    cinEdit(new QLineEdit)
{

    // Real-time validation for Nom
    connect(nomEdit, &QLineEdit::editingFinished, this, [=]() {
        QRegularExpression regex("^[A-Za-zÀ-ÿ\s'-]+$");
        if (!regex.match(nomEdit->text()).hasMatch()) {
            QMessageBox::warning(this, "Invalid Input", "Nom must contain only alphabetic characters.");
            nomEdit->setFocus();
        }
    });

    // Real-time validation for Prenom
    connect(prenomEdit, &QLineEdit::editingFinished, this, [=]() {
        QRegularExpression regex("^[A-Za-zÀ-ÿ\s'-]+$");
        if (!regex.match(prenomEdit->text()).hasMatch()) {
            QMessageBox::warning(this, "Invalid Input", "Prénom must contain only alphabetic characters.");
            prenomEdit->setFocus();
        }
    });

    // Real-time validation for CIN
    connect(cinEdit, &QLineEdit::editingFinished, this, [=]() {
        QRegularExpression regex("^[01][0-9]{7}$"); // Starts with 0 or 1, exactly 8 digits
        if (!regex.match(cinEdit->text()).hasMatch()) {
            QMessageBox::warning(this, "Invalid Input", "CIN must be exactly 8 digits starting with 0 or 1.");
            cinEdit->setFocus();
        }
    });

    // Real-time validation for Telephone
    connect(telephoneEdit, &QLineEdit::editingFinished, this, [=]() {
        QRegularExpression regex("^\\+?[0-9]{8,15}$");
        if (!regex.match(telephoneEdit->text()).hasMatch()) {
            QMessageBox::warning(this, "Invalid Input", "Téléphone must be 8 to 15 digits, optionally starting with +.");
            telephoneEdit->setFocus();
        }
    });
    setWindowTitle("Add Employee");

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Nom:", nomEdit);
    formLayout->addRow("Mail:", mailEdit);
    formLayout->addRow("Poste:", posteEdit);
    formLayout->addRow("Prenom:", prenomEdit);
    formLayout->addRow("Date d'embauche:", dateEmbaucheEdit);
    formLayout->addRow("Role:", roleEdit);
    formLayout->addRow("Presence actuelle:", presenceActuelleEdit);
    formLayout->addRow("Statut:", statutEdit);
    formLayout->addRow("Telephone:", telephoneEdit);
    formLayout->addRow("CIN:", cinEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

QString AddEmployeeDialog::getNom() const {
    return nomEdit->text();
}

QString AddEmployeeDialog::getMail() const {
    return mailEdit->text();
}

QString AddEmployeeDialog::getPoste() const {
    return posteEdit->text();
}

QString AddEmployeeDialog::getPrenom() const {
    return prenomEdit->text();
}

QString AddEmployeeDialog::getDateEmbauche() const {
    return dateEmbaucheEdit->text();
}

QString AddEmployeeDialog::getRole() const {
    return roleEdit->text();
}

QString AddEmployeeDialog::getPresenceActuelle() const {
    return presenceActuelleEdit->text();
}

QString AddEmployeeDialog::getStatut() const {
    return statutEdit->text();
}

QString AddEmployeeDialog::getTelephone() const {
    return telephoneEdit->text();
}

QString AddEmployeeDialog::getCin() const {
    return cinEdit->text();
}
UpdateEmployeeDialog::UpdateEmployeeDialog(QWidget *parent, const QString &cin, const QString &nom,
                                           const QString &mail, const QString &poste, const QString &prenom,
                                           const QString &dateEmbauche, const QString &role,
                                           const QString &presenceActuelle, const QString &statut,
                                           const QString &telephone)
    : QDialog(parent), nomEdit(new QLineEdit(nom)), mailEdit(new QLineEdit(mail)),
    posteEdit(new QLineEdit(poste)), prenomEdit(new QLineEdit(prenom)),
    dateEmbaucheEdit(new QLineEdit(dateEmbauche)), roleEdit(new QLineEdit(role)),
    presenceActuelleEdit(new QLineEdit(presenceActuelle)), statutEdit(new QLineEdit(statut)),
    telephoneEdit(new QLineEdit(telephone)), cinEdit(new QLineEdit(cin))
{

    // Real-time validation for Nom
    connect(nomEdit, &QLineEdit::editingFinished, this, [=]() {
        QRegularExpression regex("^[A-Za-zÀ-ÿ\s'-]+$");
        if (!regex.match(nomEdit->text()).hasMatch()) {
            QMessageBox::warning(this, "Invalid Input", "Nom must contain only alphabetic characters.");
            nomEdit->setFocus();
        }
    });

    // Real-time validation for Prenom
    connect(prenomEdit, &QLineEdit::editingFinished, this, [=]() {
        QRegularExpression regex("^[A-Za-zÀ-ÿ\s'-]+$");
        if (!regex.match(prenomEdit->text()).hasMatch()) {
            QMessageBox::warning(this, "Invalid Input", "Prénom must contain only alphabetic characters.");
            prenomEdit->setFocus();
        }
    });

    // Real-time validation for CIN
    connect(cinEdit, &QLineEdit::editingFinished, this, [=]() {
        QRegularExpression regex("^[01][0-9]{7}$"); // Starts with 0 or 1, exactly 8 digits
        if (!regex.match(cinEdit->text()).hasMatch()) {
            QMessageBox::warning(this, "Invalid Input", "CIN must be exactly 8 digits starting with 0 or 1.");
            cinEdit->setFocus();
        }
    });

    // Real-time validation for Telephone
    connect(telephoneEdit, &QLineEdit::editingFinished, this, [=]() {
        QRegularExpression regex("^\+?[0-9]{8,15}$");
        if (!regex.match(telephoneEdit->text()).hasMatch()) {
            QMessageBox::warning(this, "Invalid Input", "Téléphone must be 8 to 15 digits, optionally starting with +.");
            telephoneEdit->setFocus();
        }
    });
    setWindowTitle("Update Employee");

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Nom:", nomEdit);
    formLayout->addRow("Mail:", mailEdit);
    formLayout->addRow("Poste:", posteEdit);
    formLayout->addRow("Prenom:", prenomEdit);
    formLayout->addRow("Date d'embauche:", dateEmbaucheEdit);
    formLayout->addRow("Role:", roleEdit);
    formLayout->addRow("Presence actuelle:", presenceActuelleEdit);
    formLayout->addRow("Statut:", statutEdit);
    formLayout->addRow("Telephone:", telephoneEdit);
    formLayout->addRow("CIN:", cinEdit);

    // Make CIN read-only since it's the primary key
    cinEdit->setReadOnly(true);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

QString UpdateEmployeeDialog::getNom() const {
    return nomEdit->text();
}

QString UpdateEmployeeDialog::getMail() const {
    return mailEdit->text();
}

QString UpdateEmployeeDialog::getPoste() const {
    return posteEdit->text();
}

QString UpdateEmployeeDialog::getPrenom() const {
    return prenomEdit->text();
}

QString UpdateEmployeeDialog::getDateEmbauche() const {
    return dateEmbaucheEdit->text();
}

QString UpdateEmployeeDialog::getRole() const {
    return roleEdit->text();
}

QString UpdateEmployeeDialog::getPresenceActuelle() const {
    return presenceActuelleEdit->text();
}

QString UpdateEmployeeDialog::getStatut() const {
    return statutEdit->text();
}

QString UpdateEmployeeDialog::getTelephone() const {
    return telephoneEdit->text();
}

QString UpdateEmployeeDialog::getCin() const {
    return cinEdit->text();
}

AddEventDialog::AddEventDialog(QWidget *parent, const QStringList &employees)
    : QDialog(parent)
{
    setWindowTitle("Add Calendar Event");

    employeeCombo = new QComboBox;
    employeeCombo->addItems(employees);

    eventTypeCombo = new QComboBox;
    eventTypeCombo->addItems({"Vacation", "Meeting", "Working Hours", "Training", "Other"});

    startTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    startTimeEdit->setCalendarPopup(true);
    startTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm AP");

    endTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(3600)); // Default to 1 hour later
    endTimeEdit->setCalendarPopup(true);
    endTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm AP");

    descriptionEdit = new QLineEdit;

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Employee:", employeeCombo);
    formLayout->addRow("Event Type:", eventTypeCombo);
    formLayout->addRow("Start Time:", startTimeEdit);
    formLayout->addRow("End Time:", endTimeEdit);
    formLayout->addRow("Description:", descriptionEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

QString AddEventDialog::getEmployee() const {
    return employeeCombo->currentText();
}

QString AddEventDialog::getEventType() const {
    return eventTypeCombo->currentText();
}

QDateTime AddEventDialog::getStartTime() const {
    return startTimeEdit->dateTime();
}

QDateTime AddEventDialog::getEndTime() const {
    return endTimeEdit->dateTime();
}

QString AddEventDialog::getDescription() const {
    return descriptionEdit->text();
}
