#ifndef INVOICEPDFGENERATOR_H
#define INVOICEPDFGENERATOR_H

#include <QObject>
#include <QString>
#include <QPagedPaintDevice>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QDate>

class Invoice;
class Client;

/**
 * @brief The InvoicePDFGenerator class generates professional PDF invoices
 * 
 * This class creates well-formatted PDF documents from Invoice objects
 * with company branding and professional layout.
 */
class InvoicePDFGenerator : public QObject
{
    Q_OBJECT

public:
    struct CompanyInfo {
        QString name;
        QString address;
        QString phone;
        QString email;
        QString website;
        QString taxId;
        QString logoPath;
    };

    explicit InvoicePDFGenerator(QObject *parent = nullptr);
    ~InvoicePDFGenerator();

    // Configuration
    void setCompanyInfo(const CompanyInfo &info);
    CompanyInfo companyInfo() const { return m_companyInfo; }
    
    void setTemplateStyle(const QString &style);
    QString templateStyle() const { return m_templateStyle; }

    // PDF Generation
    bool generatePDF(const Invoice *invoice, const QString &filePath);
    bool generatePDF(const QList<Invoice*> &invoices, const QString &filePath);
    
    // Preview
    QTextDocument* generatePreview(const Invoice *invoice);
    
    // Utility
    QString getLastError() const { return m_lastError; }
    QStringList getAvailableTemplates() const;

public slots:
    void setOutputDirectory(const QString &directory);

signals:
    void pdfGenerated(const QString &filePath);
    void errorOccurred(const QString &error);
    void progressUpdated(int percentage);

private:
    void setupDocument(QTextDocument *document);
    void addHeader(QTextCursor &cursor, const Invoice *invoice);
    void addCompanyInfo(QTextCursor &cursor);
    void addClientInfo(QTextCursor &cursor, const Invoice *invoice);
    void addInvoiceInfo(QTextCursor &cursor, const Invoice *invoice);
    void addItemsTable(QTextCursor &cursor, const Invoice *invoice);
    void addTotals(QTextCursor &cursor, const Invoice *invoice);
    void addFooter(QTextCursor &cursor, const Invoice *invoice);
    void addLogo(QTextCursor &cursor);
    
    QString formatCurrency(double amount, const QString &currency) const;
    QString formatDate(const QDate &date) const;
    QTextCharFormat getTitleFormat() const;
    QTextCharFormat getHeaderFormat() const;
    QTextCharFormat getBodyFormat() const;
    QTextCharFormat getBoldFormat() const;
    QTextTableFormat getTableFormat() const;
    
    CompanyInfo m_companyInfo;
    QString m_templateStyle;
    QString m_outputDirectory;
    QString m_lastError;
    
    // Default styles
    static const QString DEFAULT_TEMPLATE;
    static const CompanyInfo DEFAULT_COMPANY_INFO;
};

#endif // INVOICEPDFGENERATOR_H
