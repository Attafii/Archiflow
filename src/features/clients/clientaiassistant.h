#ifndef CLIENTAIASSISTANT_H
#define CLIENTAIASSISTANT_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QScrollArea>
#include <QFrame>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include "client.h"

/**
 * @brief AI Assistant Dialog for Client Management
 * 
 * Provides AI-powered insights and chatbot functionality for client management
 * using Groq API for intelligent conversations and analysis.
 */
class ClientAIAssistant : public QDialog
{
    Q_OBJECT

public:
    explicit ClientAIAssistant(ClientContact *client = nullptr, QWidget *parent = nullptr);
    virtual ~ClientAIAssistant() = default;

    void setClient(ClientContact *client);

private slots:
    void onSendMessage();
    void onClearChat();
    void onGenerateInsights();
    void onHandleAIResponse();
    void onCopyResponse();

private:
    // UI Components
    QVBoxLayout *m_mainLayout;
    QSplitter *m_splitter;
    
    // Chat Section
    QWidget *m_chatWidget;
    QVBoxLayout *m_chatLayout;
    QScrollArea *m_chatArea;
    QWidget *m_chatContent;
    QVBoxLayout *m_chatContentLayout;
    QLineEdit *m_messageInput;
    QPushButton *m_sendBtn;
    QPushButton *m_clearBtn;
    
    // Insights Section
    QWidget *m_insightsWidget;
    QVBoxLayout *m_insightsLayout;
    QTextEdit *m_insightsDisplay;
    QPushButton *m_generateInsightsBtn;
    QPushButton *m_copyInsightsBtn;
    
    // AI Integration
    QNetworkAccessManager *m_networkManager;
    QString m_groqApiKey;
    QTimer *m_typingTimer;
    
    // Client Data
    ClientContact *m_client;
    QStringList m_chatHistory;
      // Methods
    void setupUI();
    void setupConnections();
    void setupChatSection();
    void setupInsightsSection();
    void loadEnvironmentVariables();
    void addMessageToChat(const QString &message, bool isUser = true);
    void sendToGroqAPI(const QString &prompt, const QString &context = QString());
    void processAIResponse(const QJsonObject &response);
    QString generateClientContext();
    QString generateInsightPrompt();
    QFrame* createMessageFrame(const QString &message, bool isUser);
    void scrollToBottom();
    void showTypingIndicator();
    void hideTypingIndicator();
};

#endif // CLIENTAIASSISTANT_H
