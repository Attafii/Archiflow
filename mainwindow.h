#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QtSql>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addItem();
    void deleteItem();
    void saveInvoice();
    void generatePdf();
    void sendEmail();
    void updateTotals();
    void onClientSelected(int index); // Slot to load client details into fields
    void saveClientDetails(); // NEW Slot to save/update client details

private:
    void setupUi();
    bool initDatabase();
    void loadClients();
    void loadClientDetailsToFields(int clientId); // Renamed helper function

    // Database
    QSqlDatabase db;

    // Invoice Details Widgets
    QLineEdit *invoiceNumberEdit;
    QDateEdit *dateEdit;
    QComboBox *clientComboBox;
    QPushButton *saveClientButton; // NEW button to save client edits
    QLineEdit *clientNameEdit;     // NEW editable field for client name
    QTextEdit *clientAddressEdit;  // NEW editable field for client address
    QLineEdit *clientEmailEdit;    // NEW editable field for client email
    QComboBox *paymentModeComboBox;
    QComboBox *statusComboBox;

    // Invoice Items Widgets
    QTableWidget *itemsTableWidget;
    QPushButton *addItemButton;
    QPushButton *deleteItemButton;

    // Totals Widgets
    QLineEdit *subTotalEdit;
    QLineEdit *vatEdit;
    QLineEdit *totalEdit;
    const double VAT_RATE = 0.20;

    // Action Buttons
    QPushButton *saveButton;
    QPushButton *generatePdfButton;
    QPushButton *sendEmailButton;

    // Layouts and Main Widget
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QGridLayout *detailsLayout;
    // Removed clientDetailsDisplayLayout
    QVBoxLayout *itemsLayout;
    QGridLayout *totalsLayout;
    QHBoxLayout *bottomButtonsLayout;
    QHBoxLayout *itemButtonsLayout;

};
#endif // MAINWINDOW_H

