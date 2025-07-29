#ifndef GROQCLIENT_H
#define GROQCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QQueue>

struct GroqConfig {
    QString apiKey;
    QString baseUrl = "https://api.groq.com/openai/v1";
    QString model = "llama-3.3-70b-versatile";
    int timeout = 30000;
    int maxTokens = 4096;
    double temperature = 0.7;
    bool enableRetry = true;
    int maxRetries = 3;
    int retryDelay = 1000; // milliseconds
};

struct ChatMessage {
    QString role; // "user", "assistant", "system"
    QString content;
    qint64 timestamp;
    
    ChatMessage() : timestamp(QDateTime::currentMSecsSinceEpoch()) {}
    ChatMessage(const QString &r, const QString &c) 
        : role(r), content(c), timestamp(QDateTime::currentMSecsSinceEpoch()) {}
};

class GroqClient : public QObject
{
    Q_OBJECT

public:
    explicit GroqClient(QObject *parent = nullptr);
    ~GroqClient();
    
    // Configuration
    void setConfiguration(const GroqConfig &config);
    GroqConfig configuration() const { return m_config; }
    
    // API Methods
    void sendMessage(const QString &message, const QList<ChatMessage> &context = {});
    void sendChatCompletion(const QList<ChatMessage> &messages);
    
    // Status
    bool isConnected() const { return m_isConnected; }
    bool isBusy() const { return m_pendingRequests > 0; }
    
    // Utility
    void clearContext();
    void setSystemPrompt(const QString &prompt);

public slots:
    void abortCurrentRequest();

signals:
    void messageReceived(const QString &message, const QString &messageId);
    void errorOccurred(const QString &error, int code);
    void connectionStatusChanged(bool connected);
    void requestStarted();
    void requestFinished();
    void typingStarted(); // For UI animations
    void typingFinished();

private slots:
    void handleNetworkReply();
    void handleNetworkError(QNetworkReply::NetworkError error);
    void onRequestTimeout();
    void processRetryQueue();

private:
    void setupNetworkManager();
    QJsonObject createChatCompletionRequest(const QList<ChatMessage> &messages);
    void processResponse(const QJsonDocument &response);
    void addToRetryQueue(const QJsonObject &request);
    QString generateRequestId();
    void updateConnectionStatus();
    
    GroqConfig m_config;
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_currentReply;
    QTimer *m_timeoutTimer;
    QTimer *m_retryTimer;
    
    QString m_systemPrompt;
    QList<ChatMessage> m_context;
    QQueue<QPair<QJsonObject, int>> m_retryQueue; // request, retry count
    
    bool m_isConnected;
    int m_pendingRequests;
    QString m_currentRequestId;
    
    static const QString DEFAULT_SYSTEM_PROMPT;
};

#endif // GROQCLIENT_H
