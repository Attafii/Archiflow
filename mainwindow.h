
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtSql/QSqlDatabase>
#include <QtWidgets/QCalendarWidget>
#include <QtWidgets/QStackedWidget>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QHeaderView>

class AddEmployeeDialog;
class UpdateEmployeeDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void setupUI();
    void setupEmployeePage();
    void setupCalendarTab();
    void setupDatabase();
    void loadEmployees();
    void searchEmployee();
    void deleteSelected();
    void addEmployee();
    void updateEmployee();
    void resetDatabase();

    // Calendar functionality
    void updateCalendarView();
    void addCalendarEvent();
    void deleteCalendarEvent();
    void authenticateWithGoogle();
    void fetchCalendarEvents();
    void processCalendarEvents(const QJsonArray &events);
    void updateEmployeeFilter();
    void syncEventsWithGoogle();

private:
    // Database
    QSqlDatabase db;

    // Modern UI components
    QWidget *sidebar;
    QStackedWidget *contentStack;
    QWidget *employeeTab;
    QWidget *calendarTab;

    // Employee management components
    QTableWidget *table;
    QLineEdit *searchBar;
    QComboBox *searchFilter;
    QPushButton *deleteButton;
    QPushButton *searchButton;
    QPushButton *addButton;
    QPushButton *updateButton;
    QLabel *titleLabel;

    // Calendar UI components
    QCalendarWidget *calendarWidget;
    QTableWidget *eventsTable;
    QPushButton *addEventButton;
    QPushButton *deleteEventButton;
    QComboBox *employeeFilter;
    QLabel *calendarTitle;

    // Google Calendar API
    QOAuth2AuthorizationCodeFlow *oauth2;
    QNetworkAccessManager *networkManager;
    QString calendarId;
};

class AddEmployeeDialog : public QDialog {
    Q_OBJECT

public:
    AddEmployeeDialog(QWidget *parent = nullptr);

    QString getNom() const;
    QString getMail() const;
    QString getPoste() const;
    QString getPrenom() const;
    QString getDateEmbauche() const;
    QString getRole() const;
    QString getPresenceActuelle() const;
    QString getStatut() const;
    QString getTelephone() const;
    QString getCin() const;

private:
    QLineEdit *nomEdit;
    QLineEdit *mailEdit;
    QLineEdit *posteEdit;
    QLineEdit *prenomEdit;
    QLineEdit *dateEmbaucheEdit;
    QLineEdit *roleEdit;
    QLineEdit *presenceActuelleEdit;
    QLineEdit *statutEdit;
    QLineEdit *telephoneEdit;
    QLineEdit *cinEdit;
};

class UpdateEmployeeDialog : public QDialog {
    Q_OBJECT

public:
    UpdateEmployeeDialog(QWidget *parent = nullptr, const QString &cin = "", const QString &nom = "",
                         const QString &mail = "", const QString &poste = "", const QString &prenom = "",
                         const QString &dateEmbauche = "", const QString &role = "",
                         const QString &presenceActuelle = "", const QString &statut = "",
                         const QString &telephone = "");

    QString getNom() const;
    QString getMail() const;
    QString getPoste() const;
    QString getPrenom() const;
    QString getDateEmbauche() const;
    QString getRole() const;
    QString getPresenceActuelle() const;
    QString getStatut() const;
    QString getTelephone() const;
    QString getCin() const;

private:
    QLineEdit *nomEdit;
    QLineEdit *mailEdit;
    QLineEdit *posteEdit;
    QLineEdit *prenomEdit;
    QLineEdit *dateEmbaucheEdit;
    QLineEdit *roleEdit;
    QLineEdit *presenceActuelleEdit;
    QLineEdit *statutEdit;
    QLineEdit *telephoneEdit;
    QLineEdit *cinEdit;
};

class AddEventDialog : public QDialog {
    Q_OBJECT

public:
    AddEventDialog(QWidget *parent = nullptr, const QStringList &employees = QStringList());

    QString getEmployee() const;
    QString getEventType() const;
    QDateTime getStartTime() const;
    QDateTime getEndTime() const;
    QString getDescription() const;

private:
    QComboBox *employeeCombo;
    QComboBox *eventTypeCombo;
    QDateTimeEdit *startTimeEdit;
    QDateTimeEdit *endTimeEdit;
    QLineEdit *descriptionEdit;
};

#endif // MAINWINDOW_H

