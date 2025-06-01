#include "groqaiapi.h"
#include <QDebug>
#include <QUrlQuery>

GroqAIAPI::GroqAIAPI(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

GroqAIAPI::~GroqAIAPI()
{
}

void GroqAIAPI::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}

QString GroqAIAPI::apiKey() const
{
    return m_apiKey;
}

void GroqAIAPI::requestCostEstimation(const QString &projectType, const QString &serviceArea, const QString &description, const QString &location)
{
    // Check if API key is set
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("API key is not set");
        return;
    }
    
    // Create the request URL - using Groq API endpoint
    QUrl url("https://api.groq.com/openai/v1/chat/completions");
    
    // Create the request
    QNetworkRequest request = createRequest(url);
    
    // Create the request payload
    QJsonDocument payload = createRequestPayload(projectType, serviceArea, description, location);
    
    // Send the request
    QNetworkReply *reply = m_networkManager->post(request, payload.toJson());
    
    // Connect the reply to the handler
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        this->handleNetworkReply(reply);
    });
}

QNetworkRequest GroqAIAPI::createRequest(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    // Using the same Bearer token format for Groq API
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    return request;
}

QJsonDocument GroqAIAPI::createRequestPayload(const QString &projectType, const QString &serviceArea, const QString &description, const QString &location)
{
    // Create the system message
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "You are a construction cost estimation expert. Provide a cost estimate in euros for a construction project based on the project type, service area, description, and location (if provided). Return ONLY a JSON object with two fields: 'cost' (a number representing the estimated cost in euros) and 'details' (a string explaining the estimation)."; 
    
    // Create the user message with the project details
    QJsonObject userMessage;
    userMessage["role"] = "user";
    QString content = QString("Project Type: %1\nService Area: %2 square meters\nDescription: %3").arg(projectType).arg(serviceArea).arg(description);
    
    // Add location if provided
    if (!location.isEmpty()) {
        content += QString("\nLocation: %1").arg(location);
    }
    
    content += "\n\nPlease provide a cost estimation in euros and explain the factors that influenced your estimate.";
    userMessage["content"] = content;
    
    // Create the messages array
    QJsonArray messages;
    messages.append(systemMessage);
    messages.append(userMessage);
    
    // Create the request object
    QJsonObject requestObject;
    requestObject["model"] = "compound-beta"; // Using Groq's compound-beta model
    requestObject["messages"] = messages;
    requestObject["temperature"] = 1.0;
    requestObject["max_completion_tokens"] = 1024; // Groq uses max_completion_tokens instead of max_tokens
    requestObject["top_p"] = 1.0;
    requestObject["response_format"] = QJsonObject{{"type", "json_object"}};
    
    // Create the request document
    QJsonDocument requestDocument(requestObject);
    
    return requestDocument;
}

void GroqAIAPI::handleNetworkReply(QNetworkReply *reply)
{
    // Make sure the reply is deleted when we leave this method
    reply->deleteLater();
    
    // Check for errors
    if (reply->error() != QNetworkReply::NoError) {
        QString errorDetails = QString("Network error: %1\nURL: %2\nStatus code: %3")
            .arg(reply->errorString())
            .arg(reply->url().toString())
            .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
        
        // Log the error for debugging
        qDebug() << "Groq API Error:" << errorDetails;
        qDebug() << "Response data:" << reply->readAll();
        
        emit errorOccurred(errorDetails);
        return;
    }
    
    // Read the response
    QByteArray responseData = reply->readAll();
    
    // Log the raw response for debugging
    qDebug() << "Groq API Raw Response:" << responseData;
    
    // Parse the JSON response
    QJsonDocument responseDocument = QJsonDocument::fromJson(responseData);
    if (responseDocument.isNull()) {
        emit errorOccurred("Invalid JSON response");
        return;
    }
    
    // Get the response object
    QJsonObject responseObject = responseDocument.object();
    
    // Check if the response contains choices
    if (!responseObject.contains("choices") || !responseObject["choices"].isArray()) {
        emit errorOccurred("Invalid response format: missing choices array");
        return;
    }
    
    // Get the first choice
    QJsonArray choices = responseObject["choices"].toArray();
    if (choices.isEmpty()) {
        emit errorOccurred("No choices in response");
        return;
    }
    
    // Get the message from the first choice
    QJsonObject choice = choices.first().toObject();
    if (!choice.contains("message") || !choice["message"].isObject()) {
        emit errorOccurred("Invalid choice format: missing message");
        return;
    }
    
    // Get the content from the message
    QJsonObject message = choice["message"].toObject();
    if (!message.contains("content") || !message["content"].isString()) {
        emit errorOccurred("Invalid message format: missing content");
        return;
    }
    
    // Get the content string
    QString content = message["content"].toString();
    
    // Log the content for debugging
    qDebug() << "Groq API Content:" << content;
    
    // Parse the content as JSON
    QJsonDocument contentDocument = QJsonDocument::fromJson(content.toUtf8());
    if (contentDocument.isNull()) {
        // Try to clean up the content if it's not valid JSON
        QString cleanedContent = content.trimmed();
        
        // Sometimes the model returns markdown-formatted JSON with ```json and ``` delimiters
        if (cleanedContent.startsWith("```json") || cleanedContent.startsWith("```")) {
            int startPos = cleanedContent.indexOf('{');
            int endPos = cleanedContent.lastIndexOf('}');
            
            if (startPos >= 0 && endPos > startPos) {
                cleanedContent = cleanedContent.mid(startPos, endPos - startPos + 1);
                contentDocument = QJsonDocument::fromJson(cleanedContent.toUtf8());
                
                if (contentDocument.isNull()) {
                    qDebug() << "Failed to parse cleaned content:" << cleanedContent;
                    emit errorOccurred("Invalid JSON in content (even after cleaning)");
                    return;
                }
            } else {
                qDebug() << "Could not find valid JSON object in content:" << cleanedContent;
                emit errorOccurred("Invalid JSON in content (could not find valid JSON object)");
                return;
            }
        } else {
            qDebug() << "Invalid JSON in content:" << content;
            emit errorOccurred("Invalid JSON in content");
            return;
        }
    }
    
    // Get the content object
    QJsonObject contentObject = contentDocument.object();
    
    // Check if the content object contains cost and details
    if (!contentObject.contains("cost") || !contentObject.contains("details")) {
        qDebug() << "Missing cost or details in content:" << contentDocument.toJson();
        emit errorOccurred("Invalid content format: missing cost or details");
        return;
    }
    
    // Get the cost and details
    double cost = contentObject["cost"].toDouble();
    QString details = contentObject["details"].toString();
    
    // Emit the cost estimation received signal
    emit costEstimationReceived(cost, details);
}