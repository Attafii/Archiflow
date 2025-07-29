#ifndef BASEDIALOG_H
#define BASEDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

/**
 * @brief The BaseDialog class - Base class for all dialogs
 * 
 * Provides common functionality and consistent styling for all dialogs
 * in the ArchiFlow application.
 */
class BaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BaseDialog(QWidget *parent = nullptr);
    virtual ~BaseDialog() = default;

    // Layout management
    void setMainWidget(QWidget *widget);
    void addButton(const QString &text, QDialogButtonBox::ButtonRole role);
    QPushButton* addButton(QDialogButtonBox::StandardButton button);

    // Dialog configuration
    void setTitle(const QString &title);
    void setMinimumDialogSize(const QSize &size);

protected:
    // Virtual methods for subclasses
    virtual void setupUi();
    virtual bool validateInput() { return true; }
    virtual void onAccepted() {}
    virtual void onRejected() {}
    
    // UI components accessible to subclasses
    QVBoxLayout *m_mainLayout;
    QWidget *m_contentWidget;
    QDialogButtonBox *m_buttonBox;

protected slots:
    void accept() override;
    void reject() override;

private:
    void setupBaseLayout();
};

#endif // BASEDIALOG_H
