#include "groqclient.h"
#include <QDebug>
#include <QDateTime>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QUuid>
#include <QCoreApplication>

const QString GroqClient::DEFAULT_SYSTEM_PROMPT = 
    "You are an AI assistant specialized in materials management for architecture and construction. "
    "You help users with inventory management, material specifications, cost analysis, supplier information, "
    "and construction project planning. Provide clear, concise, and professional assistance.";

GroqClient::GroqClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_currentReply(nullptr)
    , m_timeoutTimer(new QTimer(this))
    , m_retryTimer(new QTimer(this))
    , m_isConnected(false)
    , m_pendingRequests(0)
{
    setupNetworkManager();
    
    // Setup timers
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &GroqClient::onRequestTimeout);
    
    m_retryTimer->setSingleShot(true);
    connect(m_retryTimer, &QTimer::timeout, this, &GroqClient::processRetryQueue);
    
    // Set default system prompt
    m_systemPrompt = DEFAULT_SYSTEM_PROMPT;
    
    // Initialize default configuration
    m_config.apiKey = ""; // User needs to set this
}

GroqClient::~GroqClient()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
}

void GroqClient::setupNetworkManager()
{
    m_networkManager = new QNetworkAccessManager(this);
    
    // Set user agent
    QNetworkRequest request;
    request.setRawHeader("User-Agent", "ArchiFlow-Materials/1.0");
}

void GroqClient::setConfiguration(const GroqConfig &config)
{
    m_config = config;
    updateConnectionStatus();
}

void GroqClient::sendMessage(const QString &message, const QList<ChatMessage> &context)
{
    if (message.trimmed().isEmpty()) {
        emit errorOccurred("Message cannot be empty", 400);
        return;
    }
    
    if (m_config.apiKey.isEmpty()) {
        emit errorOccurred("API key not configured. Please set your Groq API key in settings.", 401);
        return;
    }
    
    QList<ChatMessage> messages = context;
    
    // Add system prompt if not present
    if (messages.isEmpty() || messages.first().role != "system") {
        messages.prepend(ChatMessage("system", m_systemPrompt));
    }
    
    // Add user message
    messages.append(ChatMessage("user", message));
    
    sendChatCompletion(messages);
}

void GroqClient::sendChatCompletion(const QList<ChatMessage> &messages)
{
    if (m_pendingRequests >= 3) { // Limit concurrent requests
        emit errorOccurred("Too many concurrent requests", 429);
        return;
    }
    
    QJsonObject request = createChatCompletionRequest(messages);
    
    QNetworkRequest networkRequest;
    networkRequest.setUrl(QUrl(m_config.baseUrl + "/chat/completions"));
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    networkRequest.setRawHeader("Authorization", ("Bearer " + m_config.apiKey).toUtf8());
    networkRequest.setRawHeader("User-Agent", "ArchiFlow-Materials/1.0");
    
    QJsonDocument doc(request);
    QByteArray data = doc.toJson();
    
    emit requestStarted();
    emit typingStarted();
    
    m_currentReply = m_networkManager->post(networkRequest, data);
    m_currentRequestId = generateRequestId();
    m_pendingRequests++;
    
    connect(m_currentReply, &QNetworkReply::finished, this, &GroqClient::handleNetworkReply);
    connect(m_currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &GroqClient::handleNetworkError);
    
    // Start timeout timer
    m_timeoutTimer->start(m_config.timeout);
    
    qDebug() << "Groq API request sent:" << m_config.baseUrl + "/chat/completions";
}

QJsonObject GroqClient::createChatCompletionRequest(const QList<ChatMessage> &messages)
{
    QJsonObject request;
    request["model"] = m_config.model;
    request["temperature"] = m_config.temperature;
    request["max_tokens"] = m_config.maxTokens;
    request["stream"] = false;
    
    QJsonArray messagesArray;
    for (const ChatMessage &msg : messages) {
        QJsonObject messageObj;
        messageObj["role"] = msg.role;
        messageObj["content"] = msg.content;
        messagesArray.append(messageObj);
    }
    request["messages"] = messagesArray;
    
    return request;
}

void GroqClient::handleNetworkReply()
{
    if (!m_currentReply) return;
    
    m_timeoutTimer->stop();
    m_pendingRequests--;
    
    if (m_currentReply->error() == QNetworkReply::NoError) {
        QByteArray data = m_currentReply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        
        if (parseError.error == QJsonParseError::NoError) {
            processResponse(doc);
        } else {
            emit errorOccurred("Failed to parse API response: " + parseError.errorString(), 500);
        }
    }
    
    emit requestFinished();
    emit typingFinished();
    
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
    
    updateConnectionStatus();
}

void GroqClient::processResponse(const QJsonDocument &response)
{
    QJsonObject root = response.object();
    
    if (root.contains("error")) {
        QJsonObject error = root["error"].toObject();
        QString errorMessage = error["message"].toString();
        int errorCode = error["code"].toInt();
        emit errorOccurred("API Error: " + errorMessage, errorCode);
        return;
    }
    
    if (root.contains("choices")) {
        QJsonArray choices = root["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject firstChoice = choices[0].toObject();
            QJsonObject message = firstChoice["message"].toObject();
            QString content = message["content"].toString();
            
            emit messageReceived(content, m_currentRequestId);
            
            // Add to context
            m_context.append(ChatMessage("assistant", content));
            
            // Limit context size to prevent token overflow
            while (m_context.size() > 20) {
                m_context.removeFirst();
            }
        }
    } else {
        emit errorOccurred("Unexpected API response format", 500);
    }
}

void GroqClient::handleNetworkError(QNetworkReply::NetworkError error)
{
    m_timeoutTimer->stop();
    m_pendingRequests--;
    
    QString errorMessage;
    int errorCode = static_cast<int>(error);
    
    switch (error) {
    case QNetworkReply::ConnectionRefusedError:
        errorMessage = "Connection refused. Please check your internet connection.";
        break;
    case QNetworkReply::TimeoutError:
        errorMessage = "Request timed out. Please try again.";
        break;
    case QNetworkReply::AuthenticationRequiredError:
        errorMessage = "Authentication failed. Please check your API key.";
        break;
    case QNetworkReply::ContentNotFoundError:
        errorMessage = "API endpoint not found.";
        break;
    case QNetworkReply::InternalServerError:
        errorMessage = "Server error. Please try again later.";
        break;
    default:
        if (m_currentReply) {
            QByteArray data = m_currentReply->readAll();
            if (!data.isEmpty()) {
                QJsonDocument doc = QJsonDocument::fromJson(data);
                QJsonObject obj = doc.object();
                if (obj.contains("error")) {
                    errorMessage = obj["error"].toObject()["message"].toString();
                } else {
                    errorMessage = QString("Network error: %1").arg(data.left(200));
                }
            } else {
                errorMessage = QString("Network error (code: %1)").arg(errorCode);
            }
        } else {
            errorMessage = QString("Network error (code: %1)").arg(errorCode);
        }
        break;
    }
    
    emit errorOccurred(errorMessage, errorCode);
    emit requestFinished();
    emit typingFinished();
    
    updateConnectionStatus();
}

void GroqClient::onRequestTimeout()
{
    if (m_currentReply) {
        m_currentReply->abort();
        emit errorOccurred("Request timed out", 408);
    }
}

void GroqClient::abortCurrentRequest()
{
    if (m_currentReply) {
        m_currentReply->abort();
    }
    m_timeoutTimer->stop();
    m_retryQueue.clear();
}

void GroqClient::clearContext()
{
    m_context.clear();
}

void GroqClient::setSystemPrompt(const QString &prompt)
{
    m_systemPrompt = prompt.isEmpty() ? DEFAULT_SYSTEM_PROMPT : prompt;
}

void GroqClient::addToRetryQueue(const QJsonObject &request)
{
    m_retryQueue.enqueue(qMakePair(request, 0));
    if (!m_retryTimer->isActive()) {
        m_retryTimer->start(m_config.retryDelay);
    }
}

void GroqClient::processRetryQueue()
{
    if (m_retryQueue.isEmpty()) return;
    
    auto [request, retryCount] = m_retryQueue.dequeue();
    
    if (retryCount < m_config.maxRetries) {
        // TODO: Implement retry logic
        qDebug() << "Retrying request, attempt:" << (retryCount + 1);
    }
    
    if (!m_retryQueue.isEmpty()) {
        m_retryTimer->start(m_config.retryDelay);
    }
}

QString GroqClient::generateRequestId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void GroqClient::updateConnectionStatus()
{
    bool wasConnected = m_isConnected;
    m_isConnected = !m_config.apiKey.isEmpty();
    
    if (wasConnected != m_isConnected) {
        emit connectionStatusChanged(m_isConnected);
    }
}
