#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "chatbotmanager.h"
#include "csvimportdialog.h"
#include <QMessageBox>
#include <QLocale>
#include <QDate>
#include <QPrinter>
#include <QPainter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QPdfWriter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , dbManager(new DatabaseManager(this))
    , expirationTimer(new QTimer(this))
{
    ui->setupUi(this);

    // Initialize database
    if (!dbManager->initializeDatabase()) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to initialize database"));
        return;
    }

    // Configure expiration check timer (check every hour)
    connect(expirationTimer, &QTimer::timeout, this, &MainWindow::checkExpiringContracts);
    expirationTimer->start(3600000); // 1 hour in milliseconds
    
    // Initial check for expiring contracts
    checkExpiringContracts();

    // Initialize table
    ui->contractTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->contractTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->contractTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Disable edit/delete buttons initially
    ui->editButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);

    // Set table headers to stretch
    ui->contractTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Configure value spin box
    ui->valueSpinBox->setPrefix("$");
    ui->valueSpinBox->setDecimals(2);

    // Setup UI components
    setupDateEdits();
    setupFilterWidgets();
    
    // Load existing contracts
    loadContractsFromDatabase();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupDateEdits()
{
    ui->startDateEdit->setCalendarPopup(true);
    ui->endDateEdit->setCalendarPopup(true);
    ui->startDateEdit->setDate(QDate::currentDate());
    ui->endDateEdit->setDate(QDate::currentDate().addYears(1));
    
    // Add chat button
    QPushButton* chatButton = new QPushButton("💬 AI Assistant", this);
    chatButton->setStyleSheet("QPushButton { background-color: #3D485A; color: #E3C6B0; border-radius: 8px; padding: 8px 16px; font-weight: bold; } QPushButton:hover { background-color: #4A5568; }");
    ui->toolBar->addWidget(chatButton);
    connect(chatButton, &QPushButton::clicked, this, &MainWindow::openChatbotDialog);
    
    // Add import button
    QPushButton* importButton = new QPushButton("📊 Import Contracts", this);
    importButton->setStyleSheet("QPushButton { background-color: #E3C6B0; color: #3D485A; border-radius: 8px; padding: 8px 16px; font-weight: bold; } QPushButton:hover { background-color: #C4A491; }");
    ui->toolBar->addWidget(importButton);
    connect(importButton, &QPushButton::clicked, this, &MainWindow::openCsvImportDialog);
}

void MainWindow::on_addButton_clicked()
{
    clearDetails();
    ui->contractIdLineEdit->setText(generateContractId());
    ui->saveButton->setEnabled(true);
}

void MainWindow::on_editButton_clicked()
{
    if (ui->contractTableWidget->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a contract to edit.");
        return;
    }
    ui->saveButton->setEnabled(true);
}

void MainWindow::on_statisticsButton_clicked()
{
    StatisticsDialog *statsDialog = new StatisticsDialog(dbManager, this);
    statsDialog->exec();
}

void MainWindow::on_exportPdfButton_clicked()
{
    exportContractsToPdf();
}

void MainWindow::on_actionExportPdf_triggered()
{
    exportContractsToPdf();
}

void MainWindow::exportContractsToPdf()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to PDF"), "", tr("PDF Files (*.pdf)"));
    if (fileName.isEmpty())
        return;
    
    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive))
        fileName += ".pdf";
    
    QPdfWriter pdfWriter(fileName);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageMargins(QMarginsF(40, 40, 40, 40));
    
    QPainter painter(&pdfWriter);
    painter.setPen(Qt::black);
    
    QFont titleFont("Arial", 24, QFont::Bold);
    QFont subtitleFont("Arial", 16, QFont::Bold);
    QFont headerFont("Arial", 12, QFont::Bold);
    QFont contentFont("Arial", 10);
    
    painter.setFont(titleFont);
    painter.drawText(QRect(0, 60, pdfWriter.width(), 50), Qt::AlignHCenter, "Contract Management");
    
    painter.setFont(subtitleFont);
    painter.drawText(QRect(0, 120, pdfWriter.width(), 40), Qt::AlignHCenter, "Active Contracts List");
    
    int tableTop = 200;
    int tableWidth = pdfWriter.width() - 80;
    int rowHeight = 35;
    int colCount = 6;
    int colWidth = tableWidth / colCount;
    
    QStringList headers;
    headers << "ID" << "Client" << "Start Date" << "End Date" << "Value" << "Status";
    
    int currentRow = 0;
    int currentY = tableTop;
    
    auto drawTableHeader = [&](int yPos) {
        QRect tableRect(40, yPos, tableWidth, rowHeight);
        painter.setPen(QPen(Qt::black, 2));
        painter.drawRect(tableRect);
        
        painter.setFont(headerFont);
        painter.setPen(QPen(Qt::black, 1));
        
        for (int i = 0; i < headers.size(); ++i) {
            QRect headerRect(40 + i * colWidth, yPos, colWidth, rowHeight);
            painter.fillRect(headerRect, QColor(240, 240, 240));
            painter.drawRect(headerRect);
            painter.drawText(headerRect, Qt::AlignCenter, headers[i]);
        }
    };
    
    drawTableHeader(tableTop);
    painter.setFont(contentFont);
    
    for (int row = 0; row < ui->contractTableWidget->rowCount(); ++row) {
        currentY = tableTop + (currentRow + 1) * rowHeight;
        
        if (currentY + rowHeight > pdfWriter.height() - 100) {
            pdfWriter.newPage();
            tableTop = 100;
            currentY = tableTop;
            currentRow = 0;
            drawTableHeader(tableTop);
            painter.setFont(contentFont);
        }
        
        painter.fillRect(QRect(40, currentY, tableWidth, rowHeight),
                         currentRow % 2 == 0 ? QColor(252, 252, 252) : QColor(245, 245, 245));
        
        for (int col = 0; col < headers.size(); ++col) {
            QRect cellRect(40 + col * colWidth, currentY, colWidth, rowHeight);
            painter.drawRect(cellRect);
            
            QString text = ui->contractTableWidget->item(row, col)->text();
            if (col == 0) {
                painter.drawText(cellRect, Qt::AlignCenter, text);
            } else if (col == 4) {
                painter.drawText(cellRect.adjusted(5, 0, -5, 0), Qt::AlignRight | Qt::AlignVCenter, text);
            } else {
                painter.drawText(cellRect.adjusted(5, 0, -5, 0), Qt::AlignLeft | Qt::AlignVCenter, text);
            }
        }
        
        currentRow++;
    }
    
    painter.end();
    QMessageBox::information(this, tr("PDF Export"), tr("Contracts have been successfully exported to PDF."));
}

void MainWindow::openChatbotDialog()
{
    ChatbotManager* chatbotManager = new ChatbotManager(dbManager, this);
    
    // Connect the database change signal to refresh the main window
    connect(chatbotManager, &ChatbotManager::databaseChanged, 
            this, &MainWindow::loadContractsFromDatabase);
    
    ChatbotDialog* chatbotDialog = new ChatbotDialog(chatbotManager, this);
    chatbotDialog->setAttribute(Qt::WA_DeleteOnClose);
    chatbotDialog->exec(); // Use exec() for modal dialog
}

void MainWindow::openCsvImportDialog()
{
    CsvImportDialog* csvDialog = new CsvImportDialog(dbManager, this);
    
    // Connect to refresh the main window after import
    connect(csvDialog, &QDialog::accepted, this, &MainWindow::loadContractsFromDatabase);
    
    csvDialog->setAttribute(Qt::WA_DeleteOnClose);
    csvDialog->exec();
}

void MainWindow::on_deleteButton_clicked()
{
    if (ui->contractTableWidget->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a contract to delete.");
        return;
    }

    QString contractId = ui->contractTableWidget->item(ui->contractTableWidget->currentRow(), 0)->text();
    
    if (QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete this contract?")
        == QMessageBox::Yes) {
        if (dbManager->deleteContract(contractId)) {
            loadContractsFromDatabase();
            clearDetails();
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete contract");
        }
    }
}

void MainWindow::on_saveButton_clicked()
{
    // Basic validation
    if (ui->contractIdLineEdit->text().isEmpty() || ui->clientNameLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Contract ID and Client Name are required fields.");
        return;
    }

    if (ui->startDateEdit->date() > ui->endDateEdit->date()) {
        QMessageBox::warning(this, "Warning", "Start date cannot be later than end date.");
        return;
    }

    QString contractId = ui->contractIdLineEdit->text();
    QString clientName = ui->clientNameLineEdit->text();
    QDate startDate = ui->startDateEdit->date();
    QDate endDate = ui->endDateEdit->date();
    double value = ui->valueSpinBox->value();
    QString status = ui->statusComboBox->currentText();
    QString description = ui->descriptionTextEdit->toPlainText();

    bool success;
    if (ui->contractTableWidget->currentRow() < 0) {
        // Add new contract
        success = dbManager->addContract(contractId, clientName, startDate, endDate,
                                       value, status, description);
    } else {
        // Update existing contract
        success = dbManager->updateContract(contractId, clientName, startDate, endDate,
                                         value, status, description);
    }

    if (success) {
        loadContractsFromDatabase();
        clearDetails();
    } else {
        QMessageBox::critical(this, "Error", "Failed to save contract");
    }
}

void MainWindow::updateContractRow(int row)
{
    ui->contractTableWidget->setItem(row, 0, new QTableWidgetItem(ui->contractIdLineEdit->text()));
    ui->contractTableWidget->setItem(row, 1, new QTableWidgetItem(ui->clientNameLineEdit->text()));
    ui->contractTableWidget->setItem(row, 2, new QTableWidgetItem(ui->startDateEdit->date().toString("yyyy-MM-dd")));
    ui->contractTableWidget->setItem(row, 3, new QTableWidgetItem(ui->endDateEdit->date().toString("yyyy-MM-dd")));
    ui->contractTableWidget->setItem(row, 4, new QTableWidgetItem(formatValue(ui->valueSpinBox->value())));
    ui->contractTableWidget->setItem(row, 5, new QTableWidgetItem(ui->statusComboBox->currentText()));
}

void MainWindow::on_contractTableWidget_itemClicked(QTableWidgetItem *item)
{
    ui->editButton->setEnabled(true);
    ui->deleteButton->setEnabled(true);
    loadContractDetails(item->row());
}

void MainWindow::clearDetails()
{
    ui->contractIdLineEdit->clear();
    ui->clientNameLineEdit->clear();
    ui->startDateEdit->setDate(QDate::currentDate());
    ui->endDateEdit->setDate(QDate::currentDate().addYears(1));
    ui->valueSpinBox->setValue(0.0);
    ui->statusComboBox->setCurrentIndex(0);
    ui->descriptionTextEdit->clear();
    ui->contractTableWidget->clearSelection();
    ui->editButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
}

void MainWindow::loadContractDetails(int row)
{
    ui->contractIdLineEdit->setText(ui->contractTableWidget->item(row, 0)->text());
    ui->clientNameLineEdit->setText(ui->contractTableWidget->item(row, 1)->text());
    ui->startDateEdit->setDate(QDate::fromString(ui->contractTableWidget->item(row, 2)->text(), "yyyy-MM-dd"));
    ui->endDateEdit->setDate(QDate::fromString(ui->contractTableWidget->item(row, 3)->text(), "yyyy-MM-dd"));

    QString valueText = ui->contractTableWidget->item(row, 4)->text();
    valueText.remove("$");
    ui->valueSpinBox->setValue(valueText.toDouble());

    ui->statusComboBox->setCurrentText(ui->contractTableWidget->item(row, 5)->text());
}

void MainWindow::on_searchLineEdit_textChanged(const QString &arg1)
{
    filterContracts();
}

void MainWindow::on_filterComboBox_currentIndexChanged(int index)
{
    filterContracts();
}

void MainWindow::setupFilterWidgets()
{
    // Setup status filter combo box
    ui->filterComboBox->addItem("All");
    ui->filterComboBox->addItems({"Active", "Pending", "Completed", "Expired"});
    
    // Setup value range filters
    ui->minValueSpinBox->setPrefix("$");
    ui->minValueSpinBox->setDecimals(2);
    ui->minValueSpinBox->setMaximum(999999999.99);
    
    ui->maxValueSpinBox->setPrefix("$");
    ui->maxValueSpinBox->setDecimals(2);
    ui->maxValueSpinBox->setMaximum(999999999.99);
    
    // Setup date range filters
    ui->startDateFilter->setCalendarPopup(true);
    ui->endDateFilter->setCalendarPopup(true);
    
    // Connect filter change signals
    connect(ui->minValueSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::filterContracts);
    connect(ui->maxValueSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::filterContracts);
    connect(ui->startDateFilter, &QDateEdit::dateChanged,
            this, &MainWindow::filterContracts);
    connect(ui->endDateFilter, &QDateEdit::dateChanged,
            this, &MainWindow::filterContracts);
}

void MainWindow::loadContractsFromDatabase()
{
    ui->contractTableWidget->setRowCount(0);
    
    QSqlQuery query = dbManager->getAllContracts();
    while (query.next()) {
        int row = ui->contractTableWidget->rowCount();
        ui->contractTableWidget->insertRow(row);
        
        ui->contractTableWidget->setItem(row, 0, new QTableWidgetItem(query.value("contract_id").toString()));
        ui->contractTableWidget->setItem(row, 1, new QTableWidgetItem(query.value("client_name").toString()));
        ui->contractTableWidget->setItem(row, 2, new QTableWidgetItem(QDate::fromString(query.value("start_date").toString(), Qt::ISODate).toString("MM/dd/yyyy")));
        ui->contractTableWidget->setItem(row, 3, new QTableWidgetItem(QDate::fromString(query.value("end_date").toString(), Qt::ISODate).toString("MM/dd/yyyy")));
        ui->contractTableWidget->setItem(row, 4, new QTableWidgetItem(formatValue(query.value("value").toDouble())));
        ui->contractTableWidget->setItem(row, 5, new QTableWidgetItem(query.value("status").toString()));
        ui->contractTableWidget->setItem(row, 6, new QTableWidgetItem(query.value("description").toString()));
    }
}

void MainWindow::filterContracts()
{
    QString searchText = ui->searchLineEdit->text();
    QString filterStatus = ui->filterComboBox->currentText();
    QDate startDate = ui->startDateFilter->date();
    QDate endDate = ui->endDateFilter->date();
    double minValue = ui->minValueSpinBox->value();
    double maxValue = ui->maxValueSpinBox->value();
    
    if (filterStatus == "All") {
        filterStatus = "";
    }
    
    QSqlQuery query = dbManager->getFilteredContracts(searchText, filterStatus,
                                                    startDate, endDate,
                                                    minValue, maxValue);
    
    ui->contractTableWidget->setRowCount(0);
    while (query.next()) {
        int row = ui->contractTableWidget->rowCount();
        ui->contractTableWidget->insertRow(row);
        
        ui->contractTableWidget->setItem(row, 0, new QTableWidgetItem(query.value("contract_id").toString()));
        ui->contractTableWidget->setItem(row, 1, new QTableWidgetItem(query.value("client_name").toString()));
        ui->contractTableWidget->setItem(row, 2, new QTableWidgetItem(QDate::fromString(query.value("start_date").toString(), Qt::ISODate).toString("MM/dd/yyyy")));
        ui->contractTableWidget->setItem(row, 3, new QTableWidgetItem(QDate::fromString(query.value("end_date").toString(), Qt::ISODate).toString("MM/dd/yyyy")));
        ui->contractTableWidget->setItem(row, 4, new QTableWidgetItem(formatValue(query.value("value").toDouble())));
        ui->contractTableWidget->setItem(row, 5, new QTableWidgetItem(query.value("status").toString()));
        ui->contractTableWidget->setItem(row, 6, new QTableWidgetItem(query.value("description").toString()));
    }

}

QString MainWindow::formatValue(double value)
{
    return QString("$%1").arg(value, 0, 'f', 2);
}

QString MainWindow::generateContractId()
{
    // Generate a unique contract ID (YYYY-NNNN format)
    QString year = QString::number(QDate::currentDate().year());
    int maxId = 0;

    // Find the highest existing ID for this year
    for(int row = 0; row < ui->contractTableWidget->rowCount(); row++) {
        QString existingId = ui->contractTableWidget->item(row, 0)->text();
        if(existingId.startsWith(year)) {
            int num = existingId.split("-").last().toInt();
            maxId = qMax(maxId, num);
        }
    }

    return QString("%1-%2").arg(year).arg(maxId + 1, 4, 10, QChar('0'));
}

void MainWindow::on_actionContractList_triggered()
{
    StatisticsDialog *dialog = new StatisticsDialog(dbManager, this);
    dialog->exec();
    delete dialog;
}

void MainWindow::on_actionExpiringContracts_triggered()
{
    QDate thirtyDaysFromNow = QDate::currentDate().addDays(30);
    QSqlQuery query = dbManager->getFilteredContracts("", "Active",
                                                    QDate(), thirtyDaysFromNow,
                                                    -1, -1);
    
    QString report = "Contracts expiring within 30 days:\n\n";
    while (query.next()) {
        report += QString("ID: %1\nClient: %2\nExpires: %3\n\n")
                 .arg(query.value("contract_id").toString())
                 .arg(query.value("client_name").toString())
                 .arg(QDate::fromString(query.value("end_date").toString(), Qt::ISODate).toString("MM/dd/yyyy"));
    }
    
    QMessageBox::information(this, "Expiring Contracts", report);
}

void MainWindow::on_actionValueSummary_triggered()
{
    double totalValue = dbManager->getTotalContractValue();
    QMap<QString, int> distribution = dbManager->getContractStatusDistribution();
    
    QString summary = QString(tr("Total Contract Value: %1\n\nContract Status Distribution:\n"))
                     .arg(formatValue(totalValue));
    
    QStringList statuses = {"Active", "Pending", "Completed", "Expired"};
    for (const QString &status : statuses) {
        summary += QString("%1: %2\n").arg(status).arg(distribution.value(status, 0));
    }
    
    QMessageBox::information(this, tr("Value Summary"), summary);
}

void MainWindow::checkExpiringContracts()
{
    QSqlQuery query = dbManager->getExpiringContracts(30); // Check contracts expiring in 30 days
    
    while (query.next()) {
        QString contractId = query.value("contract_id").toString();
        QString clientName = query.value("client_name").toString();
        QDate endDate = QDate::fromString(query.value("end_date").toString(), Qt::ISODate);
        
        handleContractExpiration(contractId, clientName, endDate);
    }
}

void MainWindow::handleContractExpiration(const QString &contractId, const QString &clientName, const QDate &endDate)
{
    int daysUntilExpiration = QDate::currentDate().daysTo(endDate);
    QString message = tr("Le contrat %1 pour le client %2 expire dans %3 jours.\n\nVoulez-vous renouveler ce contrat ?")
                     .arg(contractId)
                     .arg(clientName)
                     .arg(daysUntilExpiration);
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        tr("Alerte d'expiration de contrat"),
        message,
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Ignore);
    
    if (reply == QMessageBox::Yes) {
        // Load contract details for renewal
        for (int row = 0; row < ui->contractTableWidget->rowCount(); ++row) {
            if (ui->contractTableWidget->item(row, 0)->text() == contractId) {
                ui->contractTableWidget->selectRow(row);
                loadContractDetails(row);
                
                // Update dates for renewal
                QDate newStartDate = endDate.addDays(1);
                QDate newEndDate = newStartDate.addYears(1);
                ui->startDateEdit->setDate(newStartDate);
                ui->endDateEdit->setDate(newEndDate);
                ui->contractIdLineEdit->setText(generateContractId()); // Generate new ID
                ui->saveButton->setEnabled(true);
                break;
            }
        }
    } else if (reply == QMessageBox::No) {
        // Mark contract as expired
        for (int row = 0; row < ui->contractTableWidget->rowCount(); ++row) {
            if (ui->contractTableWidget->item(row, 0)->text() == contractId) {
                ui->contractTableWidget->selectRow(row);
                loadContractDetails(row);
                ui->statusComboBox->setCurrentText("Expired");
                on_saveButton_clicked();
                break;
            }
        }
    }
    // If Ignore, do nothing and remind again next time
}
