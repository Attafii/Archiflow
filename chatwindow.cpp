#include "chatwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateTime>

ChatWindow::ChatWindow(DatabaseManager* dbManager, QWidget *parent)
    : QWidget(parent)
    , m_dbManager(dbManager)
    , m_chatbot(new ChatbotManager(dbManager, this))
{
    setupUi();
    updateContractList();

    connect(m_chatbot, &ChatbotManager::responseReady,
            this, &ChatWindow::handleBotResponse);
    connect(m_chatbot, &ChatbotManager::errorOccurred,
            this, &ChatWindow::handleBotError);
}

ChatWindow::~ChatWindow()
{
}

void ChatWindow::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Zone de sélection du contrat
    QHBoxLayout* contractLayout = new QHBoxLayout();
    QLabel* contractLabel = new QLabel("Sélectionner un contrat:", this);
    m_contractSelector = new QComboBox(this);
    contractLayout->addWidget(contractLabel);
    contractLayout->addWidget(m_contractSelector);
    mainLayout->addLayout(contractLayout);

    // Zone d'affichage du chat
    m_chatDisplay = new QTextEdit(this);
    m_chatDisplay->setReadOnly(true);
    mainLayout->addWidget(m_chatDisplay);

    // Zone de saisie du message
    QHBoxLayout* inputLayout = new QHBoxLayout();
    m_messageInput = new QLineEdit(this);
    m_sendButton = new QPushButton("Envoyer", this);
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendButton);
    mainLayout->addLayout(inputLayout);

    // Connexions
    connect(m_sendButton, &QPushButton::clicked, this, &ChatWindow::sendMessage);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &ChatWindow::sendMessage);

    // Style et dimensions
    setMinimumSize(500, 600);
    m_chatDisplay->setStyleSheet("QTextEdit { background-color: #f0f0f0; border-radius: 5px; padding: 10px; }");
    m_messageInput->setStyleSheet("QLineEdit { border-radius: 5px; padding: 5px; }");
    m_sendButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border-radius: 5px; padding: 5px 15px; }");
}

void ChatWindow::sendMessage()
{
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty() || m_contractSelector->currentText().isEmpty()) {
        return;
    }

    displayMessage(message);
    m_messageInput->clear();

    // Envoyer la requête au chatbot
    m_chatbot->processUserRequest(message, m_contractSelector->currentText());
}

void ChatWindow::handleBotResponse(const QString &response)
{
    displayMessage(response, false);
    updateContractList(); // Mettre à jour la liste des contrats après modification
}

void ChatWindow::handleBotError(const QString &error)
{
    displayMessage("Erreur: " + error, false);
}

void ChatWindow::displayMessage(const QString &message, bool isUser)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm");
    QString htmlMessage = QString("<p style='margin: 5px 0; color: black;'><b>%1 [%2]:</b> %3</p>")
                         .arg(isUser ? "Vous" : "Assistant")
                         .arg(timestamp)
                         .arg(message);
    m_chatDisplay->append(htmlMessage);
}

void ChatWindow::updateContractList()
{
    m_contractSelector->clear();
    QSqlQuery query = m_dbManager->getAllContracts();
    while (query.next()) {
        QString contractId = query.value("contract_id").toString();
        QString clientName = query.value("client_name").toString();
        m_contractSelector->addItem(QString("%1 - %2").arg(contractId, clientName), contractId);
    }
}