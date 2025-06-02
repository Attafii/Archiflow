#include "mainwindow.h"
#include <QtWidgets>
#include <QtSql>
#include <QMessageBox>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <QPdfWriter>
#include <QPainter>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QInputDialog>
#include <QFontMetrics>
#include <QTextDocument>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    if (!initDatabase()) {
        QMessageBox::critical(this, tr("Erreur Base de Données"), tr("Impossible d\'initialiser la base de données."));
        // Disable UI elements if DB is required
        saveButton->setEnabled(false);
        clientComboBox->setEnabled(false);
        saveClientButton->setEnabled(false); // Disable client save if DB fails
        addItemButton->setEnabled(false);
        deleteItemButton->setEnabled(false);
        generatePdfButton->setEnabled(false);
        sendEmailButton->setEnabled(false);
    } else {
        loadClients(); // Load clients after DB is ready
    }

    // Connect signals and slots
    connect(addItemButton, &QPushButton::clicked, this, &MainWindow::addItem);
    connect(deleteItemButton, &QPushButton::clicked, this, &MainWindow::deleteItem);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveInvoice);
    connect(generatePdfButton, &QPushButton::clicked, this, &MainWindow::generatePdf);
    connect(sendEmailButton, &QPushButton::clicked, this, &MainWindow::sendEmail);
    connect(itemsTableWidget, &QTableWidget::cellChanged, this, &MainWindow::updateTotals);
    connect(clientComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onClientSelected);
    connect(saveClientButton, &QPushButton::clicked, this, &MainWindow::saveClientDetails); // Connect new button
}

MainWindow::~MainWindow()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool MainWindow::initDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dbPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    db.setDatabaseName(dbPath + "/invoice_app.db");

    if (!db.open()) {
        qWarning() << "Error: connection with database failed:" << db.lastError();
        return false;
    }

    QSqlQuery query;

    // Create Clients table
    if (!query.exec("CREATE TABLE IF NOT EXISTS Clients ("
                    "client_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "name TEXT NOT NULL UNIQUE,"
                    "address TEXT,"
                    "email TEXT,"
                    "phone TEXT"
                    ");")) {
        qWarning() << "Couldn\'t create the Clients table, error:" << query.lastError();
        return false;
    }
    query.exec("INSERT OR IGNORE INTO Clients (name, address, email, phone) VALUES (" 
               "\'Client Test\', \'123 Rue Exemple\\n75001 Paris\', \'test@example.com\', \'0123456789\'" 
               ");");

    // Create Invoices table
    if (!query.exec("CREATE TABLE IF NOT EXISTS Invoices ("
                    "invoice_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "client_id INTEGER,"
                    "invoice_date TEXT,"
                    "payment_mode TEXT,"
                    "status TEXT,"
                    "sub_total REAL,"
                    "vat_rate REAL,"
                    "vat_amount REAL,"
                    "total_amount REAL,"
                    "FOREIGN KEY (client_id) REFERENCES Clients (client_id)"
                    ");")) {
        qWarning() << "Couldn\'t create the Invoices table, error:" << query.lastError();
        return false;
    }

    // Create InvoiceItems table
    if (!query.exec("CREATE TABLE IF NOT EXISTS InvoiceItems ("
                    "item_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "invoice_id INTEGER,"
                    "description TEXT,"
                    "quantity REAL,"
                    "unit_price REAL,"
                    "total_price REAL,"
                    "FOREIGN KEY (invoice_id) REFERENCES Invoices (invoice_id)"
                    ");")) {
        qWarning() << "Couldn\'t create the InvoiceItems table, error:" << query.lastError();
        return false;
    }

    qInfo() << "Database initialized successfully at:" << db.databaseName();
    return true;
}

void MainWindow::loadClients()
{
    clientComboBox->clear();
    clientComboBox->addItem(tr("--- Nouveau Client --- "), -1); // Option to clear fields for new client

    QSqlQuery query("SELECT client_id, name FROM Clients ORDER BY name");
    if (!query.exec()) {
        qWarning() << "Failed to load clients:" << query.lastError();
        return;
    }

    while (query.next()) {
        int clientId = query.value(0).toInt();
        QString clientName = query.value(1).toString();
        clientComboBox->addItem(clientName, clientId);
    }
    // Trigger initial display update
    onClientSelected(clientComboBox->currentIndex());
}

// Slot: Called when client selection changes
void MainWindow::onClientSelected(int index)
{
    int clientId = clientComboBox->itemData(index).toInt();
    loadClientDetailsToFields(clientId);
}

// Helper: Fetches and populates editable client fields
void MainWindow::loadClientDetailsToFields(int clientId)
{
    if (clientId <= 0) {
        // Clear fields if 'New Client' is chosen
        clientNameEdit->clear();
        clientAddressEdit->clear();
        clientEmailEdit->clear();
        clientNameEdit->setPlaceholderText(tr("Nom du nouveau client"));
        clientAddressEdit->setPlaceholderText(tr("Adresse du nouveau client"));
        clientEmailEdit->setPlaceholderText(tr("Email du nouveau client"));
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT name, address, email FROM Clients WHERE client_id = :id");
    query.bindValue(":id", clientId);

    if (query.exec() && query.next()) {
        clientNameEdit->setText(query.value(0).toString());
        clientAddressEdit->setPlainText(query.value(1).toString()); // Use plain text for QTextEdit
        clientEmailEdit->setText(query.value(2).toString());
        clientNameEdit->setPlaceholderText(""); // Clear placeholder on load
        clientAddressEdit->setPlaceholderText("");
        clientEmailEdit->setPlaceholderText("");
    } else {
        qWarning() << "Failed to fetch details for client ID:" << clientId << query.lastError();
        QMessageBox::warning(this, tr("Erreur Chargement Client"), tr("Impossible de charger les détails pour le client sélectionné."));
        // Clear fields on error
        clientNameEdit->clear();
        clientAddressEdit->clear();
        clientEmailEdit->clear();
    }
}

// NEW Slot: Save or Update Client Details from editable fields
void MainWindow::saveClientDetails()
{
    QString name = clientNameEdit->text().trimmed();
    QString address = clientAddressEdit->toPlainText().trimmed(); // Get text from QTextEdit
    QString email = clientEmailEdit->text().trimmed();
    int currentClientId = clientComboBox->currentData().toInt();

    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Sauvegarde Client Impossible"), tr("Le nom du client ne peut pas être vide."));
        return;
    }

    QSqlQuery query;
    bool isUpdate = (currentClientId > 0);

    if (isUpdate) {
        // Update existing client
        query.prepare("UPDATE Clients SET name = :name, address = :address, email = :email WHERE client_id = :id");
        query.bindValue(":id", currentClientId);
    } else {
        // Insert new client
        query.prepare("INSERT INTO Clients (name, address, email) VALUES (:name, :address, :email)");
    }

    query.bindValue(":name", name);
    query.bindValue(":address", address);
    query.bindValue(":email", email);

    if (query.exec()) {
        int savedClientId = currentClientId;
        if (!isUpdate) {
            savedClientId = query.lastInsertId().toInt();
        }
        QMessageBox::information(this, tr("Client Sauvegardé"), tr("Les informations du client ont été sauvegardées avec succès."));
        // Refresh client list and select the saved client
        int previousIndex = clientComboBox->currentIndex();
        loadClients();
        int newIndex = clientComboBox->findData(savedClientId);
        if (newIndex != -1) {
            clientComboBox->setCurrentIndex(newIndex);
        } else {
             clientComboBox->setCurrentIndex(previousIndex); // Fallback if not found
        }
    } else {
        QMessageBox::critical(this, tr("Erreur Sauvegarde Client"), tr("Erreur lors de la sauvegarde du client: %1").arg(query.lastError().text()));
    }
}


void MainWindow::addItem()
{
    int newRow = itemsTableWidget->rowCount();
    itemsTableWidget->insertRow(newRow);
    itemsTableWidget->setItem(newRow, 0, new QTableWidgetItem(""));
    itemsTableWidget->setItem(newRow, 1, new QTableWidgetItem("1"));
    itemsTableWidget->setItem(newRow, 2, new QTableWidgetItem("0.00"));
    QTableWidgetItem *totalCell = new QTableWidgetItem("0.00");
    totalCell->setFlags(totalCell->flags() & ~Qt::ItemIsEditable);
    itemsTableWidget->setItem(newRow, 3, totalCell);
    updateTotals();
}

void MainWindow::deleteItem()
{
    int currentRow = itemsTableWidget->currentRow();
    if (currentRow >= 0) {
        itemsTableWidget->removeRow(currentRow);
        updateTotals();
    }
}

void MainWindow::updateTotals()
{
    double subTotal = 0.0;
    for (int row = 0; row < itemsTableWidget->rowCount(); ++row) {
        QTableWidgetItem *quantityItem = itemsTableWidget->item(row, 1);
        QTableWidgetItem *unitPriceItem = itemsTableWidget->item(row, 2);
        QTableWidgetItem *totalItem = itemsTableWidget->item(row, 3);

        if (quantityItem && unitPriceItem && totalItem) {
            bool okQuant, okPrice;
            double quantity = quantityItem->text().toDouble(&okQuant);
            double unitPrice = unitPriceItem->text().toDouble(&okPrice);

            if (okQuant && okPrice && quantity >= 0 && unitPrice >= 0) {
                double lineTotal = quantity * unitPrice;
                itemsTableWidget->blockSignals(true);
                totalItem->setText(QString::number(lineTotal, 'f', 2));
                itemsTableWidget->blockSignals(false);
                subTotal += lineTotal;
            } else {
                itemsTableWidget->blockSignals(true);
                totalItem->setText("0.00");
                itemsTableWidget->blockSignals(false);
            }
        }
    }

    double vatAmount = subTotal * VAT_RATE;
    double totalAmount = subTotal + vatAmount;

    subTotalEdit->setText(QString::number(subTotal, 'f', 2));
    vatEdit->setText(QString::number(vatAmount, 'f', 2));
    totalEdit->setText(QString::number(totalAmount, 'f', 2));
}

void MainWindow::saveInvoice()
{
    int clientId = clientComboBox->currentData().toInt();
    // Check if a client is selected OR if the fields contain a new client name
    if (clientId <= 0 && clientNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Sauvegarde impossible"), tr("Veuillez sélectionner un client existant ou saisir les informations d\'un nouveau client et le sauvegarder d\'abord."));
        return;
    }
    // If fields have data but no client ID, prompt to save client first
    if (clientId <= 0 && !clientNameEdit->text().trimmed().isEmpty()) {
         QMessageBox::warning(this, tr("Sauvegarde impossible"), tr("Veuillez d\'abord cliquer sur \'Sauvegarder Client\' pour enregistrer le nouveau client avant de sauvegarder la facture."));
        return;
    }

    if (itemsTableWidget->rowCount() == 0) {
        QMessageBox::warning(this, tr("Sauvegarde impossible"), tr("Veuillez ajouter au moins un article à la facture."));
        return;
    }

    if (!db.transaction()) {
        QMessageBox::critical(this, tr("Erreur Base de Données"), tr("Impossible de démarrer la transaction: %1").arg(db.lastError().text()));
        return;
    }

    QSqlQuery invoiceQuery;
    invoiceQuery.prepare("INSERT INTO Invoices (client_id, invoice_date, payment_mode, status, sub_total, vat_rate, vat_amount, total_amount) "
                         "VALUES (:client_id, :invoice_date, :payment_mode, :status, :sub_total, :vat_rate, :vat_amount, :total_amount)");
    invoiceQuery.bindValue(":client_id", clientId);
    invoiceQuery.bindValue(":invoice_date", dateEdit->date().toString(Qt::ISODate));
    invoiceQuery.bindValue(":payment_mode", paymentModeComboBox->currentText());
    invoiceQuery.bindValue(":status", statusComboBox->currentText());
    invoiceQuery.bindValue(":sub_total", subTotalEdit->text().toDouble());
    invoiceQuery.bindValue(":vat_rate", VAT_RATE);
    invoiceQuery.bindValue(":vat_amount", vatEdit->text().toDouble());
    invoiceQuery.bindValue(":total_amount", totalEdit->text().toDouble());

    if (!invoiceQuery.exec()) {
        QMessageBox::critical(this, tr("Erreur Base de Données"), tr("Erreur lors de l\'enregistrement de la facture: %1").arg(invoiceQuery.lastError().text()));
        db.rollback();
        return;
    }

    QVariant lastId = invoiceQuery.lastInsertId();
    if (!lastId.isValid()) {
        QMessageBox::critical(this, tr("Erreur Base de Données"), tr("Impossible de récupérer l\'ID de la facture."));
        db.rollback();
        return;
    }
    int invoiceId = lastId.toInt();

    QSqlQuery itemQuery;
    itemQuery.prepare("INSERT INTO InvoiceItems (invoice_id, description, quantity, unit_price, total_price) "
                      "VALUES (:invoice_id, :description, :quantity, :unit_price, :total_price)");

    for (int row = 0; row < itemsTableWidget->rowCount(); ++row) {
        QString description = itemsTableWidget->item(row, 0)->text();
        double quantity = itemsTableWidget->item(row, 1)->text().toDouble();
        double unitPrice = itemsTableWidget->item(row, 2)->text().toDouble();
        double totalPrice = itemsTableWidget->item(row, 3)->text().toDouble();

        if (description.isEmpty() || quantity <= 0 || unitPrice < 0) {
             QMessageBox::warning(this, tr("Données invalides"), tr("Ligne %1 contient des données invalides (description vide, quantité <= 0 ou prix < 0).").arg(row + 1));
             db.rollback();
             return;
        }

        itemQuery.bindValue(":invoice_id", invoiceId);
        itemQuery.bindValue(":description", description);
        itemQuery.bindValue(":quantity", quantity);
        itemQuery.bindValue(":unit_price", unitPrice);
        itemQuery.bindValue(":total_price", totalPrice);

        if (!itemQuery.exec()) {
            QMessageBox::critical(this, tr("Erreur Base de Données"), tr("Erreur lors de l\'enregistrement de l\'article ligne %1: %2").arg(row + 1).arg(itemQuery.lastError().text()));
            db.rollback();
            return;
        }
    }

    if (!db.commit()) {
        QMessageBox::critical(this, tr("Erreur Base de Données"), tr("Impossible de valider la transaction: %1").arg(db.lastError().text()));
        db.rollback();
        return;
    }

    invoiceNumberEdit->setText(QString::number(invoiceId));
    QMessageBox::information(this, tr("Sauvegarde réussie"), tr("Facture #%1 enregistrée avec succès.").arg(invoiceId));
}

void MainWindow::generatePdf()
{
    bool ok;
    int invoiceId = invoiceNumberEdit->text().toInt(&ok);
    if (!ok || invoiceId <= 0) {
        QMessageBox::warning(this, tr("Génération PDF"), tr("Veuillez d\'abord enregistrer la facture.")); // Changed message slightly
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Enregistrer le PDF"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Facture_" + QString::number(invoiceId) + ".pdf",
                                                    tr("Fichiers PDF (*.pdf)"));
    if (fileName.isEmpty()) {
        return;
    }

    // Get client details directly from the editable fields
    QString clientName = clientNameEdit->text();
    QString clientAddress = clientAddressEdit->toPlainText().replace("\n", "<br>"); // Use field content
    QString clientEmail = clientEmailEdit->text();

    QString invoiceDate = dateEdit->date().toString("dd/MM/yyyy");
    QString paymentMode = paymentModeComboBox->currentText();
    QString status = statusComboBox->currentText();

    QString html;
    html += "<!DOCTYPE html><html><head><meta charset=\'UTF-8\'><style>";
    html += "body { font-family: Arial, sans-serif; font-size: 11pt; } ";
    html += "table { width: 100%; border-collapse: collapse; margin-bottom: 20px; } ";
    html += "th, td { border: 1px solid #cccccc; padding: 8px; text-align: left; vertical-align: top; } ";
    html += "th { background-color: #4682B4; color: white; font-weight: bold; } ";
    html += ".header-table td { border: none; padding: 0; } ";
    html += ".totals-table { width: 45%; float: right; margin-top: 20px; } ";
    html += ".totals-table td { border: none; padding: 4px 8px; } ";
    html += ".totals-table td.label { text-align: right; font-weight: bold; padding-right: 10px; } ";
    html += ".totals-table td.value { text-align: right; } ";
    html += ".total-row td { border-top: 2px solid #000000; font-weight: bold; font-size: 12pt; } ";
    html += "h1 { font-size: 22pt; margin-bottom: 10px; color: #4682B4; } ";
    html += ".client-details { margin-bottom: 30px; } ";
    html += ".footer { font-size: 9pt; color: #777777; margin-top: 50px; text-align: center; } ";
    html += "</style></head><body>";

    html += "<table class=\'header-table\'><tr>";
    html += QString("<td><h1>%1</h1></td>").arg(tr("FACTURE"));
    html += QString("<td style=\'text-align: right; vertical-align: bottom;\'><b>%1 #%2</b><br>%3: %4</td>")
                .arg(tr("Facture")).arg(invoiceId)
                .arg(tr("Date")).arg(invoiceDate);
    html += "</tr></table>";

    html += "<table class=\'header-table client-details\'><tr>";
    html += "<td style=\'vertical-align: top; width: 50%;\'>";
    html += QString("<b>%1:</b><br>").arg(tr("Client"));
    html += clientName.toHtmlEscaped() + "<br>";
    if (!clientAddress.isEmpty()) html += clientAddress.toHtmlEscaped() + "<br>"; // Already has <br>
    if (!clientEmail.isEmpty()) html += clientEmail.toHtmlEscaped() + "<br>";
    html += "</td>";
    html += "<td style=\'vertical-align: top; text-align: right; width: 50%;\'>";
    html += QString("<b>%1:</b> %2<br>").arg(tr("Mode de paiement")).arg(paymentMode.toHtmlEscaped());
    html += QString("<b>%1:</b> %2<br>").arg(tr("Statut")).arg(status.toHtmlEscaped());
    html += "</td>";
    html += "</tr></table>";

    html += "<table><thead><tr>";
    html += QString("<th>%1</th>").arg(tr("Description"));
    html += QString("<th style=\'text-align: right;\'>%1</th>").arg(tr("Quantité"));
    html += QString("<th style=\'text-align: right;\'>%1</th>").arg(tr("Prix unit."));
    html += QString("<th style=\'text-align: right;\'>%1</th>").arg(tr("Total"));
    html += "</tr></thead><tbody>";

    for (int row = 0; row < itemsTableWidget->rowCount(); ++row) {
        QString desc = itemsTableWidget->item(row, 0)->text();
        QString qty = itemsTableWidget->item(row, 1)->text();
        QString price = QString::number(itemsTableWidget->item(row, 2)->text().toDouble(), 'f', 2);
        QString total = QString::number(itemsTableWidget->item(row, 3)->text().toDouble(), 'f', 2);
        html += "<tr>";
        html += QString("<td>%1</td>").arg(desc.toHtmlEscaped());
        html += QString("<td style=\'text-align: right;\'>%1</td>").arg(qty.toHtmlEscaped());
        html += QString("<td style=\'text-align: right;\'>%1</td>").arg(price.toHtmlEscaped());
        html += QString("<td style=\'text-align: right;\'>%1</td>").arg(total.toHtmlEscaped());
        html += "</tr>";
    }
    int minRows = 8;
    for (int i = itemsTableWidget->rowCount(); i < minRows; ++i) {
        html += "<tr><td>&nbsp;</td><td></td><td></td><td></td></tr>";
    }
    html += "</tbody></table>";

    html += "<table class=\'totals-table\'><tbody>";
    html += QString("<tr><td class=\'label\'>%1:</td><td class=\'value\'>%2</td></tr>")
                .arg(tr("Sous-total")).arg(subTotalEdit->text());
    html += QString("<tr><td class=\'label\'>%1 (%2%):</td><td class=\'value\'>%3</td></tr>")
                .arg(tr("TVA")).arg(VAT_RATE * 100).arg(vatEdit->text());
    html += QString("<tr class=\'total-row\'><td class=\'label\'>%1:</td><td class=\'value\'>%2</td></tr>")
                .arg(tr("Total")).arg(totalEdit->text());
    html += "</tbody></table>";

    html += QString("<p class=\'footer\'>%1</p>").arg(tr("Généré par Facture Architecte Intelligent"));

    html += "</body></html>";

    QPdfWriter pdfWriter(fileName);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageMargins(QMarginsF(30, 30, 30, 30));
    pdfWriter.setTitle(tr("Facture #%1").arg(invoiceId));
    pdfWriter.setCreator("Facture Architecte Intelligent");

    QTextDocument document;
    document.setHtml(html);
    document.print(&pdfWriter);

    QMessageBox::information(this, tr("PDF Généré"), tr("Le fichier PDF a été enregistré avec succès:\n%1").arg(fileName));
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

void MainWindow::sendEmail()
{
    bool ok;
    int invoiceId = invoiceNumberEdit->text().toInt(&ok);
    if (!ok || invoiceId <= 0) {
        QMessageBox::warning(this, tr("Envoi Email"), tr("Veuillez d\'abord enregistrer la facture."));
        return;
    }

    QString defaultPdfPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Facture_" + QString::number(invoiceId) + ".pdf";
    if (!QFile::exists(defaultPdfPath)) {
        QMessageBox::warning(this, tr("Envoi Email"),
                               tr("Le fichier PDF (%1) n\'a pas été trouvé.\nVeuillez d\'abord générer le PDF, puis l\'attacher manuellement à votre email.").arg(defaultPdfPath));
    }

    // Get email from the editable field
    QString clientEmail = clientEmailEdit->text().trimmed();

    if (clientEmail.isEmpty()) {
        // Prompt if still empty
        clientEmail = QInputDialog::getText(this, tr("Email du Client"), tr("L\'email du client n\'est pas renseigné dans les champs. Veuillez le saisir:"));
        if (clientEmail.isEmpty()) {
            return;
        }
    }

    QUrl mailUrl("mailto:");
    mailUrl.setPath(clientEmail);
    mailUrl.setQuery("subject=" + QUrl::toPercentEncoding(tr("Facture #%1").arg(invoiceId)) +
                   "&body=" + QUrl::toPercentEncoding(tr("Bonjour,\n\nVeuillez trouver ci-joint la facture #%1.\n\n(N\'oubliez pas d\'attacher le fichier PDF : %2)\n\nCordialement,\n[Votre Nom/Société]").arg(invoiceId).arg(defaultPdfPath)));

    if (!QDesktopServices::openUrl(mailUrl)) {
        QMessageBox::warning(this, tr("Envoi Email"), tr("Impossible d\'ouvrir le client de messagerie par défaut."));
    }
}

void MainWindow::setupUi()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);

    // --- Invoice Details GroupBox ---
    QGroupBox *detailsGroup = new QGroupBox(tr("Détails de la Facture"));
    detailsLayout = new QGridLayout(detailsGroup);

    detailsLayout->addWidget(new QLabel(tr("Numéro de facture:")), 0, 0);
    invoiceNumberEdit = new QLineEdit();
    invoiceNumberEdit->setPlaceholderText(tr("Auto-généré"));
    invoiceNumberEdit->setReadOnly(true);
    detailsLayout->addWidget(invoiceNumberEdit, 0, 1);

    detailsLayout->addWidget(new QLabel(tr("Date:")), 0, 2);
    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("dd/MM/yyyy");
    detailsLayout->addWidget(dateEdit, 0, 3);

    // Client Selection and Save Button
    detailsLayout->addWidget(new QLabel(tr("Client:")), 1, 0);
    clientComboBox = new QComboBox();
    detailsLayout->addWidget(clientComboBox, 1, 1, 1, 2); // Span 2 columns
    saveClientButton = new QPushButton(tr("Sauvegarder Client"));
    detailsLayout->addWidget(saveClientButton, 1, 3); // Add save button next to dropdown

    // Editable Client Fields
    detailsLayout->addWidget(new QLabel(tr("Nom:")), 2, 0);
    clientNameEdit = new QLineEdit();
    detailsLayout->addWidget(clientNameEdit, 2, 1, 1, 3); // Span 3 columns

    detailsLayout->addWidget(new QLabel(tr("Adresse:")), 3, 0, Qt::AlignTop); // Align label top
    clientAddressEdit = new QTextEdit();
    clientAddressEdit->setFixedHeight(60); // Set a fixed height for address
    detailsLayout->addWidget(clientAddressEdit, 3, 1, 1, 3);

    detailsLayout->addWidget(new QLabel(tr("Email:")), 4, 0);
    clientEmailEdit = new QLineEdit();
    detailsLayout->addWidget(clientEmailEdit, 4, 1, 1, 3);

    // Payment Mode and Status
    detailsLayout->addWidget(new QLabel(tr("Mode de paiement:")), 5, 0);
    paymentModeComboBox = new QComboBox();
    paymentModeComboBox->addItems({tr("Espèces"), tr("Virement bancaire"), tr("Carte de crédit"), tr("Chèque")});
    detailsLayout->addWidget(paymentModeComboBox, 5, 1);

    detailsLayout->addWidget(new QLabel(tr("Statut:")), 5, 2);
    statusComboBox = new QComboBox();
    statusComboBox->addItems({tr("En attente"), tr("Payée"), tr("Annulée")});
    detailsLayout->addWidget(statusComboBox, 5, 3);

    mainLayout->addWidget(detailsGroup);

    // Removed the separate client display group

    // --- Invoice Items GroupBox ---
    QGroupBox *itemsGroup = new QGroupBox(tr("Articles de la Facture"));
    itemsLayout = new QVBoxLayout(itemsGroup);

    itemsTableWidget = new QTableWidget();
    itemsTableWidget->setColumnCount(4);
    itemsTableWidget->setHorizontalHeaderLabels({tr("Description"), tr("Quantité"), tr("Prix unitaire"), tr("Total")});
    itemsTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    itemsTableWidget->setAlternatingRowColors(true);
    itemsLayout->addWidget(itemsTableWidget);

    itemButtonsLayout = new QHBoxLayout();
    addItemButton = new QPushButton(tr("Ajouter un article"));
    deleteItemButton = new QPushButton(tr("Supprimer l\'article"));
    itemButtonsLayout->addWidget(addItemButton);
    itemButtonsLayout->addWidget(deleteItemButton);
    itemButtonsLayout->addStretch();
    itemsLayout->addLayout(itemButtonsLayout);

    mainLayout->addWidget(itemsGroup);

    // --- Totals GroupBox ---
    QGroupBox *totalsGroup = new QGroupBox(tr("Totaux"));
    totalsLayout = new QGridLayout(totalsGroup);

    totalsLayout->addWidget(new QLabel(tr("Sous-total:")), 0, 0, Qt::AlignRight);
    subTotalEdit = new QLineEdit("0.00");
    subTotalEdit->setReadOnly(true);
    subTotalEdit->setAlignment(Qt::AlignRight);
    totalsLayout->addWidget(subTotalEdit, 0, 1);

    totalsLayout->addWidget(new QLabel(tr("TVA (20%):")), 1, 0, Qt::AlignRight);
    vatEdit = new QLineEdit("0.00");
    vatEdit->setReadOnly(true);
    vatEdit->setAlignment(Qt::AlignRight);
    totalsLayout->addWidget(vatEdit, 1, 1);

    totalsLayout->addWidget(new QLabel(tr("Total:")), 2, 0, Qt::AlignRight);
    totalEdit = new QLineEdit("0.00");
    totalEdit->setReadOnly(true);
    totalEdit->setAlignment(Qt::AlignRight);
    QFont totalFont = totalEdit->font();
    totalFont.setBold(true);
    totalEdit->setFont(totalFont);
    totalsLayout->addWidget(totalEdit, 2, 1);

    totalsLayout->setColumnStretch(1, 1);

    mainLayout->addWidget(totalsGroup);

    // --- Bottom Action Buttons ---
    bottomButtonsLayout = new QHBoxLayout();
    saveButton = new QPushButton(tr("Enregistrer Facture")); // Renamed slightly
    generatePdfButton = new QPushButton(tr("Générer PDF"));
    sendEmailButton = new QPushButton(tr("Envoyer par email"));

    saveButton->setStyleSheet("background-color: #4CAF50; color: white; padding: 8px 16px; border: none; border-radius: 4px;");
    generatePdfButton->setStyleSheet("background-color: #007BFF; color: white; padding: 8px 16px; border: none; border-radius: 4px;");
    sendEmailButton->setStyleSheet("background-color: #17A2B8; color: white; padding: 8px 16px; border: none; border-radius: 4px;");
    addItemButton->setStyleSheet("background-color: #007BFF; color: white; padding: 5px 10px; border: none; border-radius: 4px;");
    deleteItemButton->setStyleSheet("background-color: #DC3545; color: white; padding: 5px 10px; border: none; border-radius: 4px;");
    saveClientButton->setStyleSheet("padding: 5px 10px;"); // Simple style for client save button

    bottomButtonsLayout->addWidget(saveButton);
    bottomButtonsLayout->addWidget(generatePdfButton);
    bottomButtonsLayout->addWidget(sendEmailButton);
    bottomButtonsLayout->addStretch();

    mainLayout->addLayout(bottomButtonsLayout);

    resize(800, 800);
}

