#include "chatbotmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QTimer>

ChatbotManager::ChatbotManager(DatabaseManager* dbManager, QObject *parent)
    : QObject(parent)
    , m_dbManager(dbManager)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_apiKey("AIzaSyDxIfZIb2q_5vrrYPS702rjPnxn2gW9Tec")
    , m_retryCount(0)
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &ChatbotManager::handleApiResponse);
}

ChatbotManager::~ChatbotManager()
{
    delete m_networkManager;
}

void ChatbotManager::processUserRequest(const QString &userInput, const QString &contractId)
{
    QString prompt = QString(
        "Analyser la demande suivante concernant un contrat : '%1'. "
        "Identifier si c'est une modification des termes de paiement ou l'ajout d'une clause. "
        "Répondre au format JSON avec les champs 'type' (payment_terms/non_compete_clause) "
        "et 'value' (nombre de jours pour paiement ou 'true' pour clause)."
    ).arg(userInput);

    sendApiRequest(prompt, contractId);
}

void ChatbotManager::sendApiRequest(const QString &prompt, const QString &contractId)
{
    retryApiRequest(prompt, 0, contractId);
}

void ChatbotManager::retryApiRequest(const QString &prompt, int attempt, const QString &contractId)
{
    QString apiUrl = QString("https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=%1").arg(m_apiKey);
    QNetworkRequest request{QUrl{apiUrl}};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject jsonBody;
    QJsonArray contents;
    QJsonObject content;
    QJsonArray parts;
    QJsonObject part;
    part["text"] = prompt;
    parts.append(part);
    content["parts"] = parts;
    contents.append(content);
    jsonBody["contents"] = contents;
    
    QJsonDocument doc(jsonBody);
    
    QNetworkReply* reply = m_networkManager->post(request, doc.toJson());
    reply->setProperty("attempt", attempt);
    reply->setProperty("prompt", prompt);
    reply->setProperty("contractId", contractId);
    
    qDebug() << "Sending API request:" << prompt;
    qDebug() << "Request body:" << doc.toJson();
}

void ChatbotManager::handleApiResponse(QNetworkReply *reply)
{
    int attempt = reply->property("attempt").toInt();
    QString prompt = reply->property("prompt").toString();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        qDebug() << "API Response:" << response;
        
        QJsonDocument jsonResponse = QJsonDocument::fromJson(response);
        QJsonObject jsonObject = jsonResponse.object();
        
        if (jsonResponse.isNull()) {
            emit errorOccurred("Invalid JSON response");
            reply->deleteLater();
            return;
        }
        
        // More flexible response parsing
        if (jsonObject.contains("candidates")) {
            QJsonArray candidates = jsonObject["candidates"].toArray();
            if (!candidates.isEmpty()) {
                QJsonObject firstCandidate = candidates.first().toObject();
                if (firstCandidate.contains("content")) {
                    QJsonObject content = firstCandidate["content"].toObject();
                    if (content.contains("parts")) {
                        QJsonArray parts = content["parts"].toArray();
                        if (!parts.isEmpty()) {
                            QString responseText = parts.first().toObject()["text"].toString();
                            QString contractId = reply->property("contractId").toString();
                            parseAndExecuteCommand(responseText, contractId);
                            reply->deleteLater();
                            return;
                        }
                    }
                }
            }
        }
        
        emit errorOccurred("Unexpected API response format");
    } else {
        if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
            handleAuthenticationError();
        }
        
        if (attempt < MAX_RETRIES) {
            QString contractId = reply->property("contractId").toString();
            scheduleRetry(prompt, attempt + 1, contractId);
            reply->deleteLater();
            return;
        }
        
        QString errorMessage = "Erreur API : " + reply->errorString();
        if (attempt == MAX_RETRIES) {
            errorMessage += " (Nombre maximum de tentatives atteint)";
        }
        emit errorOccurred(errorMessage);
    }
    
    reply->deleteLater();
}

void ChatbotManager::handleAuthenticationError()
{
    emit errorOccurred("Erreur d'authentification avec l'API. Veuillez vérifier votre clé API.");
}

void ChatbotManager::scheduleRetry(const QString &prompt, int attempt, const QString &contractId)
{
    int delay = INITIAL_RETRY_DELAY * (1 << (attempt - 1)); // Backoff exponentiel
    QTimer::singleShot(delay, this, [this, prompt, attempt, contractId]() {
        retryApiRequest(prompt, attempt, contractId);
    });
}

void ChatbotManager::updatePaymentTerms(const QString &contractId, int days)
{
    QSqlQuery query = m_dbManager->getAllContracts();
    while (query.next()) {
        if (query.value("contract_id").toString() == contractId) {
            QString description = query.value("description").toString();
            description += QString("\nTermes de paiement modifiés à %1 jours").arg(days);
            
            m_dbManager->updateContract(
                contractId,
                query.value("client_name").toString(),
                query.value("start_date").toDate(),
                query.value("end_date").toDate(),
                query.value("value").toDouble(),
                query.value("status").toString(),
                description
            );
            
            emit responseReady(QString("Les termes de paiement ont été mis à jour à %1 jours.").arg(days));
            return;
        }
    }
    emit errorOccurred("Contrat non trouvé.");
}

void ChatbotManager::addNonCompeteClause(const QString &contractId)
{
    QSqlQuery query = m_dbManager->getAllContracts();
    while (query.next()) {
        if (query.value("contract_id").toString() == contractId) {
            QString description = query.value("description").toString();
            description += "\nClause de non-concurrence ajoutée : Le client s'engage à ne pas exercer ";
            description += "d'activités concurrentes pendant une durée de 2 ans après la fin du contrat.";
            
            m_dbManager->updateContract(
                contractId,
                query.value("client_name").toString(),
                query.value("start_date").toDate(),
                query.value("end_date").toDate(),
                query.value("value").toDouble(),
                query.value("status").toString(),
                description
            );
            
            emit responseReady("La clause de non-concurrence a été ajoutée au contrat.");
            return;
        }
    }
    emit errorOccurred("Contrat non trouvé.");
}

void ChatbotManager::parseAndExecuteCommand(const QString &apiResponse, const QString &contractId)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(apiResponse.toUtf8());
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        emit errorOccurred("Format de réponse invalide");
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    QString type = jsonObj["type"].toString();
    
    if (type == "payment_terms") {
        int days = jsonObj["value"].toInt();
        updatePaymentTerms(contractId, days);
    } else if (type == "non_compete_clause") {
        addNonCompeteClause(contractId);
    } else {
        emit errorOccurred("Type de modification non reconnu");
    }
}