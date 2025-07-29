#include "invoicepdfgenerator.h"
#include "invoice.h"
#include "invoiceitem.h"

#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QFont>
#include <QColor>
#include <QPrinter>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QPixmap>
#include <QTextImageFormat>

// Static defaults
const QString InvoicePDFGenerator::DEFAULT_TEMPLATE = "professional";

const InvoicePDFGenerator::CompanyInfo InvoicePDFGenerator::DEFAULT_COMPANY_INFO = {
    "ArchiFlow Solutions",
    "123 Business Street\nCity, State 12345\nCountry",
    "+1 (555) 123-4567",
    "info@archiflow.com",
    "www.archiflow.com",
    "TAX123456789",
    ""
};

InvoicePDFGenerator::InvoicePDFGenerator(QObject *parent)
    : QObject(parent)
    , m_companyInfo(DEFAULT_COMPANY_INFO)
    , m_templateStyle(DEFAULT_TEMPLATE)
    , m_outputDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
{
}

InvoicePDFGenerator::~InvoicePDFGenerator()
{
}

void InvoicePDFGenerator::setCompanyInfo(const CompanyInfo &info)
{
    m_companyInfo = info;
}

void InvoicePDFGenerator::setTemplateStyle(const QString &style)
{
    m_templateStyle = style;
}

void InvoicePDFGenerator::setOutputDirectory(const QString &directory)
{
    m_outputDirectory = directory;
}

bool InvoicePDFGenerator::generatePDF(const Invoice *invoice, const QString &filePath)
{
    if (!invoice) {
        m_lastError = tr("Invalid invoice provided");
        emit errorOccurred(m_lastError);
        return false;
    }
    
    try {
        emit progressUpdated(10);
        
        // Create printer
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(filePath);
        printer.setPageSize(QPageSize::A4);
        printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);
        
        emit progressUpdated(30);
        
        // Create document
        QTextDocument document;
        setupDocument(&document);
        
        QTextCursor cursor(&document);
        
        emit progressUpdated(50);
        
        // Build document content
        addHeader(cursor, invoice);
        addCompanyInfo(cursor);
        addClientInfo(cursor, invoice);
        addInvoiceInfo(cursor, invoice);
        addItemsTable(cursor, invoice);
        addTotals(cursor, invoice);
        addFooter(cursor, invoice);
        
        emit progressUpdated(80);
        
        // Print to PDF
        document.print(&printer);
        
        emit progressUpdated(100);
        emit pdfGenerated(filePath);
        
        return true;
    }
    catch (const std::exception &e) {
        m_lastError = tr("PDF generation failed: %1").arg(e.what());
        emit errorOccurred(m_lastError);
        return false;
    }
    catch (...) {
        m_lastError = tr("Unknown error occurred during PDF generation");
        emit errorOccurred(m_lastError);
        return false;
    }
}

bool InvoicePDFGenerator::generatePDF(const QList<Invoice*> &invoices, const QString &filePath)
{
    if (invoices.isEmpty()) {
        m_lastError = tr("No invoices provided");
        emit errorOccurred(m_lastError);
        return false;
    }
    
    try {
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(filePath);
        printer.setPageSize(QPageSize::A4);
        printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);
        
        QTextDocument document;
        setupDocument(&document);
        QTextCursor cursor(&document);
        
        int totalInvoices = invoices.size();
        for (int i = 0; i < totalInvoices; ++i) {
            const Invoice *invoice = invoices[i];
            if (!invoice) continue;
            
            if (i > 0) {
                cursor.insertBlock();
                cursor.insertText("\n\n"); // Page break equivalent
            }
            
            addHeader(cursor, invoice);
            addCompanyInfo(cursor);
            addClientInfo(cursor, invoice);
            addInvoiceInfo(cursor, invoice);
            addItemsTable(cursor, invoice);
            addTotals(cursor, invoice);
            addFooter(cursor, invoice);
            
            emit progressUpdated((i + 1) * 100 / totalInvoices);
        }
        
        document.print(&printer);
        emit pdfGenerated(filePath);
        
        return true;
    }
    catch (const std::exception &e) {
        m_lastError = tr("Batch PDF generation failed: %1").arg(e.what());
        emit errorOccurred(m_lastError);
        return false;
    }
    catch (...) {
        m_lastError = tr("Unknown error occurred during batch PDF generation");
        emit errorOccurred(m_lastError);
        return false;
    }
}

QTextDocument* InvoicePDFGenerator::generatePreview(const Invoice *invoice)
{
    if (!invoice) {
        m_lastError = tr("Invalid invoice provided for preview");
        return nullptr;
    }
    
    QTextDocument *document = new QTextDocument();
    setupDocument(document);
    
    QTextCursor cursor(document);
    
    addHeader(cursor, invoice);
    addCompanyInfo(cursor);
    addClientInfo(cursor, invoice);
    addInvoiceInfo(cursor, invoice);
    addItemsTable(cursor, invoice);
    addTotals(cursor, invoice);
    addFooter(cursor, invoice);
    
    return document;
}

QStringList InvoicePDFGenerator::getAvailableTemplates() const
{
    return QStringList() << "professional" << "modern" << "classic" << "minimal";
}

void InvoicePDFGenerator::setupDocument(QTextDocument *document)
{
    document->setDefaultFont(QFont("Arial", 10));
    document->setPageSize(QSizeF(595, 842)); // A4 size in points
}

void InvoicePDFGenerator::addHeader(QTextCursor &cursor, const Invoice *invoice)
{
    Q_UNUSED(invoice)
    
    // Add logo if available
    if (!m_companyInfo.logoPath.isEmpty() && QFileInfo::exists(m_companyInfo.logoPath)) {
        addLogo(cursor);
    }
    
    // Invoice title
    cursor.insertText(tr("INVOICE"), getTitleFormat());
    cursor.insertBlock();
    cursor.insertBlock();
}

void InvoicePDFGenerator::addCompanyInfo(QTextCursor &cursor)
{
    cursor.insertText(m_companyInfo.name, getHeaderFormat());
    cursor.insertBlock();
    
    QStringList addressLines = m_companyInfo.address.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : addressLines) {
        cursor.insertText(line, getBodyFormat());
        cursor.insertBlock();
    }
    
    if (!m_companyInfo.phone.isEmpty()) {
        cursor.insertText(tr("Phone: %1").arg(m_companyInfo.phone), getBodyFormat());
        cursor.insertBlock();
    }
    
    if (!m_companyInfo.email.isEmpty()) {
        cursor.insertText(tr("Email: %1").arg(m_companyInfo.email), getBodyFormat());
        cursor.insertBlock();
    }
    
    if (!m_companyInfo.website.isEmpty()) {
        cursor.insertText(tr("Web: %1").arg(m_companyInfo.website), getBodyFormat());
        cursor.insertBlock();
    }
    
    cursor.insertBlock();
}

void InvoicePDFGenerator::addClientInfo(QTextCursor &cursor, const Invoice *invoice)
{
    cursor.insertText(tr("Bill To:"), getHeaderFormat());
    cursor.insertBlock();
    
    cursor.insertText(invoice->clientName(), getBoldFormat());
    cursor.insertBlock();
    
    if (!invoice->clientAddress().isEmpty()) {
        QStringList addressLines = invoice->clientAddress().split('\n', Qt::SkipEmptyParts);
        for (const QString &line : addressLines) {
            cursor.insertText(line, getBodyFormat());
            cursor.insertBlock();
        }
    }
    
    if (!invoice->clientEmail().isEmpty()) {
        cursor.insertText(invoice->clientEmail(), getBodyFormat());
        cursor.insertBlock();
    }
    
    if (!invoice->clientPhone().isEmpty()) {
        cursor.insertText(invoice->clientPhone(), getBodyFormat());
        cursor.insertBlock();
    }
    
    cursor.insertBlock();
}

void InvoicePDFGenerator::addInvoiceInfo(QTextCursor &cursor, const Invoice *invoice)
{
    // Create a simple table for invoice info
    QTextTable *infoTable = cursor.insertTable(4, 2, getTableFormat());
    
    // Invoice Number
    QTextTableCell cell = infoTable->cellAt(0, 0);
    QTextCursor cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(tr("Invoice Number:"), getBoldFormat());
    
    cell = infoTable->cellAt(0, 1);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(invoice->invoiceNumber(), getBodyFormat());
    
    // Invoice Date
    cell = infoTable->cellAt(1, 0);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(tr("Invoice Date:"), getBoldFormat());
    
    cell = infoTable->cellAt(1, 1);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(formatDate(invoice->invoiceDate()), getBodyFormat());
    
    // Due Date
    cell = infoTable->cellAt(2, 0);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(tr("Due Date:"), getBoldFormat());
    
    cell = infoTable->cellAt(2, 1);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(formatDate(invoice->dueDate()), getBodyFormat());
    
    // Status
    cell = infoTable->cellAt(3, 0);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(tr("Status:"), getBoldFormat());
    
    cell = infoTable->cellAt(3, 1);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(invoice->status(), getBodyFormat());
    
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertBlock();
}

void InvoicePDFGenerator::addItemsTable(QTextCursor &cursor, const Invoice *invoice)
{
    const QList<InvoiceItem*> items = invoice->items();
    if (items.isEmpty()) return;
    
    // Create items table
    QTextTable *itemsTable = cursor.insertTable(items.size() + 1, 5, getTableFormat());
    
    // Header row
    QStringList headers = {tr("Description"), tr("Quantity"), tr("Unit"), tr("Unit Price"), tr("Total")};
    for (int col = 0; col < headers.size(); ++col) {
        QTextTableCell cell = itemsTable->cellAt(0, col);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(headers[col], getBoldFormat());
    }
    
    // Data rows
    for (int row = 0; row < items.size(); ++row) {
        const InvoiceItem *item = items[row];
        
        // Description
        QTextTableCell cell = itemsTable->cellAt(row + 1, 0);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(item->description(), getBodyFormat());
        
        // Quantity
        cell = itemsTable->cellAt(row + 1, 1);
        cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(QString::number(item->quantity()), getBodyFormat());
        
        // Unit
        cell = itemsTable->cellAt(row + 1, 2);
        cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(item->unit(), getBodyFormat());
        
        // Unit Price
        cell = itemsTable->cellAt(row + 1, 3);
        cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(formatCurrency(item->unitPrice(), invoice->currency()), getBodyFormat());
        
        // Total
        cell = itemsTable->cellAt(row + 1, 4);
        cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(formatCurrency(item->totalPrice(), invoice->currency()), getBodyFormat());
    }
    
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
}

void InvoicePDFGenerator::addTotals(QTextCursor &cursor, const Invoice *invoice)
{
    // Create totals table
    QTextTable *totalsTable = cursor.insertTable(3, 2, getTableFormat());
    
    // Subtotal
    QTextTableCell cell = totalsTable->cellAt(0, 0);
    QTextCursor cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(tr("Subtotal:"), getBoldFormat());
    
    cell = totalsTable->cellAt(0, 1);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(formatCurrency(invoice->subtotal(), invoice->currency()), getBodyFormat());
    
    // Tax
    cell = totalsTable->cellAt(1, 0);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(tr("Tax (%1%):").arg(invoice->taxRate()), getBoldFormat());
    
    cell = totalsTable->cellAt(1, 1);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(formatCurrency(invoice->taxAmount(), invoice->currency()), getBodyFormat());
    
    // Total
    cell = totalsTable->cellAt(2, 0);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(tr("Total Amount:"), getBoldFormat());
    
    cell = totalsTable->cellAt(2, 1);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(formatCurrency(invoice->totalAmount(), invoice->currency()), getBoldFormat());
    
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertBlock();
}

void InvoicePDFGenerator::addFooter(QTextCursor &cursor, const Invoice *invoice)
{
    if (!invoice->notes().isEmpty()) {
        cursor.insertText(tr("Notes:"), getBoldFormat());
        cursor.insertBlock();
        cursor.insertText(invoice->notes(), getBodyFormat());
        cursor.insertBlock();
        cursor.insertBlock();
    }
    
    // Payment terms or footer info
    cursor.insertText(tr("Thank you for your business!"), getBodyFormat());
    cursor.insertBlock();
    
    if (!m_companyInfo.taxId.isEmpty()) {
        cursor.insertText(tr("Tax ID: %1").arg(m_companyInfo.taxId), getBodyFormat());
        cursor.insertBlock();
    }
}

void InvoicePDFGenerator::addLogo(QTextCursor &cursor)
{
    QPixmap logo(m_companyInfo.logoPath);
    if (!logo.isNull()) {
        // Scale logo to reasonable size
        logo = logo.scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        
        QTextImageFormat imageFormat;
        imageFormat.setName(m_companyInfo.logoPath);
        imageFormat.setWidth(logo.width());
        imageFormat.setHeight(logo.height());
        
        cursor.insertImage(imageFormat);
        cursor.insertBlock();
    }
}

QString InvoicePDFGenerator::formatCurrency(double amount, const QString &currency) const
{
    QString symbol = "$";
    if (currency == "EUR") symbol = "€";
    else if (currency == "GBP") symbol = "£";
    else if (currency == "CAD") symbol = "C$";
    else if (currency == "AUD") symbol = "A$";
    
    return QString("%1%2").arg(symbol).arg(amount, 0, 'f', 2);
}

QString InvoicePDFGenerator::formatDate(const QDate &date) const
{
    return date.toString("MMMM d, yyyy");
}

QTextCharFormat InvoicePDFGenerator::getTitleFormat() const
{
    QTextCharFormat format;
    format.setFont(QFont("Arial", 24, QFont::Bold));
    format.setForeground(QColor("#0066CC"));
    return format;
}

QTextCharFormat InvoicePDFGenerator::getHeaderFormat() const
{
    QTextCharFormat format;
    format.setFont(QFont("Arial", 14, QFont::Bold));
    format.setForeground(QColor("#333333"));
    return format;
}

QTextCharFormat InvoicePDFGenerator::getBodyFormat() const
{
    QTextCharFormat format;
    format.setFont(QFont("Arial", 10));
    format.setForeground(QColor("#666666"));
    return format;
}

QTextCharFormat InvoicePDFGenerator::getBoldFormat() const
{
    QTextCharFormat format;
    format.setFont(QFont("Arial", 10, QFont::Bold));
    format.setForeground(QColor("#333333"));
    return format;
}

QTextTableFormat InvoicePDFGenerator::getTableFormat() const
{
    QTextTableFormat format;
    format.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    format.setBorder(1);
    format.setCellPadding(5);
    format.setCellSpacing(0);
    format.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    return format;
}
