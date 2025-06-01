#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QComboBox>
#include "chatbotmanager.h"
#include "databasemanager.h"

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(DatabaseManager* dbManager, QWidget *parent = nullptr);
    ~ChatWindow();

private slots:
    void sendMessage();
    void handleBotResponse(const QString &response);
    void handleBotError(const QString &error);
    void updateContractList();

private:
    void setupUi();
    void displayMessage(const QString &message, bool isUser = true);

    QTextEdit* m_chatDisplay;
    QLineEdit* m_messageInput;
    QPushButton* m_sendButton;
    QComboBox* m_contractSelector;
    ChatbotManager* m_chatbot;
    DatabaseManager* m_dbManager;
};

#endif // CHATWINDOW_H