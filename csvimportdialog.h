#ifndef CSVIMPORTDIALOG_H
#define CSVIMPORTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include "databasemanager.h"

class CsvImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CsvImportDialog(DatabaseManager* dbManager, QWidget *parent = nullptr);
    ~CsvImportDialog();

private slots:
    void browseFile();
    void importContracts();

private:
    void setupUI();
    bool validateCsvFile(const QString &filePath);
    int processContracts(const QString &filePath);
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    QLabel *m_titleLabel;
    QTextEdit *m_instructionsText;
    QHBoxLayout *m_fileLayout;
    QLineEdit *m_filePathEdit;
    QPushButton *m_browseButton;
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_importButton;
    QPushButton *m_cancelButton;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    
    // Data
    DatabaseManager *m_dbManager;
    
    // Styling constants
    static const QString DIALOG_STYLE;
    static const QString BUTTON_STYLE;
    static const QString INPUT_STYLE;
    static const QString INSTRUCTIONS_STYLE;
};

#endif // CSVIMPORTDIALOG_H
