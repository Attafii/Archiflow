#include "basedialog.h"
#include <QLabel>

BaseDialog::BaseDialog(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_contentWidget(nullptr)
    , m_buttonBox(nullptr)
{
    setupBaseLayout();
    setupUi();
}

void BaseDialog::setupBaseLayout()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    m_mainLayout->setSpacing(12);

    // Content area
    m_contentWidget = new QWidget(this);
    m_mainLayout->addWidget(m_contentWidget);

    // Button box
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_mainLayout->addWidget(m_buttonBox);

    // Connect signals
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &BaseDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &BaseDialog::reject);

    // Set default properties
    setModal(true);
    resize(400, 300);
}

void BaseDialog::setMainWidget(QWidget *widget)
{
    if (!widget) {
        return;
    }

    // Remove existing content
    if (m_contentWidget) {
        m_mainLayout->removeWidget(m_contentWidget);
        m_contentWidget->deleteLater();
    }

    // Set new content
    m_contentWidget = widget;
    m_mainLayout->insertWidget(0, m_contentWidget);
}

void BaseDialog::addButton(const QString &text, QDialogButtonBox::ButtonRole role)
{
    m_buttonBox->addButton(text, role);
}

QPushButton* BaseDialog::addButton(QDialogButtonBox::StandardButton button)
{
    return m_buttonBox->addButton(button);
}

void BaseDialog::setTitle(const QString &title)
{
    setWindowTitle(title);
}

void BaseDialog::setMinimumDialogSize(const QSize &size)
{
    setMinimumSize(size);
}

void BaseDialog::setupUi()
{
    // Default implementation - subclasses should override
}

void BaseDialog::accept()
{
    if (validateInput()) {
        onAccepted();
        QDialog::accept();
    }
}

void BaseDialog::reject()
{
    onRejected();
    QDialog::reject();
}
