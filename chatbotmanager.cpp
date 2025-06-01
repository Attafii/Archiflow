#include "chatbotmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QTimer>
#include <QRegularExpression>
#include <QDateTime>
#include <QSqlQuery>

ChatbotManager::ChatbotManager(DatabaseManager* dbManager, QObject *parent)
    : QObject(parent)
    , m_dbManager(dbManager)
    , m_networkManager(new QNetworkAccessManager(this))
    // Directly using the API key provided by the user.
    // REMINDER: This is a security risk for shared/production code.
    , m_apiKey("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")
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
        "You are an intelligent assistant for a contract management application with full CRUD, statistics, and search capabilities. "
        "IMPORTANT: Always respond with ONLY valid JSON format, no additional text or markdown. "
        "Analyze the following request: '%1'. "
        "\n=== AVAILABLE OPERATIONS ===\n"
        "\n📋 CREATE OPERATIONS:\n"
        "   - 'create contract for ABC Corp' → {'type': 'create_contract', 'client_name': 'ABC Corp'}\n"
        "   - 'add new contract for XYZ with value 50000' → {'type': 'create_contract', 'client_name': 'XYZ', 'value': 50000}\n"
        "   - 'create contract for Company A starting 2025-07-01 ending 2026-07-01 value 75000 status Active' → {'type': 'create_contract', 'client_name': 'Company A', 'start_date': '2025-07-01', 'end_date': '2026-07-01', 'value': 75000, 'status': 'Active'}\n"
        "\n📝 READ/SEARCH OPERATIONS:\n"
        "   - 'what are the available contracts', 'list contracts', 'show all contracts' → {'type': 'search_contracts'}\n"
        "   - 'show me active contracts' → {'type': 'search_contracts', 'status': 'Active'}\n"
        "   - 'find contracts for ABC Corp' → {'type': 'search_contracts', 'search_term': 'ABC Corp'}\n"
        "   - 'show completed contracts' → {'type': 'search_contracts', 'status': 'Completed'}\n"
        "   - 'search for contracts containing project' → {'type': 'search_contracts', 'search_term': 'project'}\n"
        "\n✏️ UPDATE OPERATIONS:\n"
        "   - 'change contract ID:120 client name to NewClient' → {'type': 'update_contract', 'contract_id': '120', 'field': 'client_name', 'value': 'NewClient'}\n"
        "   - 'update contract with client name \"pii\" to client name \"hello\"' → {'type': 'update_contract', 'search_by': 'client_name', 'search_value': 'pii', 'field': 'client_name', 'value': 'hello'}\n"
        "   - 'change contract 2025-0003 status to Completed' → {'type': 'update_contract', 'contract_id': '2025-0003', 'field': 'status', 'value': 'Completed'}\n"
        "   - 'update contract 120 value to 25000' → {'type': 'update_contract', 'contract_id': '120', 'field': 'value', 'value': '25000'}\n"
        "   - 'modify payment terms to 30 days' (requires selected contract) → {'type': 'payment_terms', 'value': 30}\n"
        "\n🗑️ DELETE OPERATIONS:\n"
        "   - 'delete contract ID:120' → {'type': 'delete_contract', 'contract_id': '120'}\n"
        "   - 'remove contract 2025-0003' → {'type': 'delete_contract', 'contract_id': '2025-0003'}\n"
        "\n📊 STATISTICS OPERATIONS:\n"
        "   - 'show statistics', 'get contract stats', 'contract summary' → {'type': 'get_contract_stats'}\n"
        "   - 'total contract value', 'how much are all contracts worth' → {'type': 'get_contract_stats'}\n"
        "\n⚠️ EXPIRING CONTRACTS:\n"
        "   - 'contracts expiring in 30 days' → {'type': 'expiring_contracts', 'days': 30}\n"
        "   - 'which contracts expire soon' → {'type': 'expiring_contracts', 'days': 60}\n"
        "\n🔧 CONTRACT MODIFICATIONS:\n"
        "   - 'add non-compete clause' (requires selected contract) → {'type': 'non_compete_clause'}\n"
        "\n💬 GENERAL CONVERSATION:\n"
        "   - 'hello', 'hi', 'how are you', 'what can you do' → {'type': 'general', 'response': 'your helpful response'}\n"
        "\nRESPOND WITH ONLY THE JSON OBJECT, NO OTHER TEXT."
    ).arg(userInput);

    sendApiRequest(prompt, contractId);
}

void ChatbotManager::sendApiRequest(const QString &prompt, const QString &contractId)
{
    retryApiRequest(prompt, 0, contractId);
}

void ChatbotManager::retryApiRequest(const QString &prompt, int attempt, const QString &contractId)
{
    // Groq API endpoint for chat completions
    QString apiUrl = QString("https://api.groq.com/openai/v1/chat/completions");
    QNetworkRequest request{QUrl{apiUrl}};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    // IMPORTANT: Ensure m_apiKey is set with your actual Groq API key.
    request.setRawHeader("Authorization", QByteArray("Bearer ") + m_apiKey.toUtf8());

    QJsonObject jsonBody;
    QJsonArray messagesArray;
    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = prompt; // The prompt from processUserRequest
    messagesArray.append(userMessage);    jsonBody["messages"] = messagesArray;
    jsonBody["model"] = "compound-beta"; // As specified in the cURL command
    jsonBody["temperature"] = 1.0;
    jsonBody["max_tokens"] = 1024; // Groq uses max_tokens
    jsonBody["top_p"] = 1.0;
    jsonBody["stream"] = false;
    // For "stop": null, QJsonValue::Null can be used if the API requires the key to be present.
    // If omitting the key implies null, that's also an option.
    // The cURL example explicitly sends null.
    jsonBody["stop"] = QJsonValue(QJsonValue::Null);


    QJsonDocument doc(jsonBody);

    QNetworkReply* reply = m_networkManager->post(request, doc.toJson());
    reply->setProperty("attempt", attempt);
    reply->setProperty("prompt", prompt); // Keep for retry logic if needed
    reply->setProperty("contractId", contractId); // Keep for context

    qDebug() << "Sending API request to Groq:" << prompt;
    qDebug() << "Request body:" << doc.toJson(QJsonDocument::Compact);
}

void ChatbotManager::handleApiResponse(QNetworkReply *reply)
{
    int attempt = reply->property("attempt").toInt();
    QString prompt = reply->property("prompt").toString(); // For retry context

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        qDebug() << "Groq API Response:" << responseData;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        if (jsonResponse.isNull() || !jsonResponse.isObject()) {
            emit errorOccurred("Invalid JSON response from Groq API");
            reply->deleteLater();
            return;
        }

        QJsonObject jsonObject = jsonResponse.object();

        if (jsonObject.contains("choices")) {
            QJsonArray choices = jsonObject["choices"].toArray();
            if (!choices.isEmpty()) {
                QJsonObject firstChoice = choices.first().toObject();
                if (firstChoice.contains("message")) {
                    QJsonObject message = firstChoice["message"].toObject();
                    if (message.contains("content")) {
                        QString responseText = message["content"].toString();
                        QString contractId = reply->property("contractId").toString();
                        // Assuming parseAndExecuteCommand is still relevant
                        parseAndExecuteCommand(responseText, contractId);
                        reply->deleteLater();
                        return;
                    }
                }
            }
        }
        // Fallthrough if the expected structure isn't found
        emit errorOccurred("Unexpected Groq API response format (missing choices/message/content). Response: " + QString(responseData));

    } else {
        qDebug() << "Groq API Error:" << reply->errorString() << "HTTP Status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray errorBody = reply->readAll(); // Read error body if any
        qDebug() << "Groq API Error Body:" << errorBody;

        if (reply->error() == QNetworkReply::AuthenticationRequiredError ||
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 401) { // Unauthorized
            handleAuthenticationError(); // This will emit "Erreur d'authentification...
        } else if (attempt < MAX_RETRIES) {
            QString contractId = reply->property("contractId").toString();
            scheduleRetry(prompt, attempt + 1, contractId);
            reply->deleteLater();
            return;
        } else {
            QString errorMessage = "Groq API Error: " + reply->errorString();
            if (!errorBody.isEmpty()) {
                errorMessage += " - Details: " + QString(errorBody);
            }
            if (attempt >= MAX_RETRIES) {
                errorMessage += " (Maximum retry attempts reached)";
            }
            emit errorOccurred(errorMessage);
        }
    }

    reply->deleteLater();
}

void ChatbotManager::handleAuthenticationError()
{
    emit errorOccurred("Authentication error with the API. Please check your API key.");
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
            QString description = query.value("description").toString();            description += QString("\nPayment terms modified to %1 days").arg(days);
            
            m_dbManager->updateContract(
                contractId,
                query.value("client_name").toString(),
                query.value("start_date").toDate(),
                query.value("end_date").toDate(),
                query.value("value").toDouble(),
                query.value("status").toString(),
                description
            );
              emit responseReady(QString("Payment terms have been updated to %1 days.").arg(days));
            return;
        }
    }
    emit errorOccurred("Contract not found.");
}

void ChatbotManager::addNonCompeteClause(const QString &contractId)
{
    QSqlQuery query = m_dbManager->getAllContracts();
    while (query.next()) {
        if (query.value("contract_id").toString() == contractId) {
            QString description = query.value("description").toString();            description += "\nNon-compete clause added: The client agrees not to engage ";
            description += "in competing activities for a period of 2 years after the end of the contract.";
            
            m_dbManager->updateContract(
                contractId,
                query.value("client_name").toString(),
                query.value("start_date").toDate(),
                query.value("end_date").toDate(),
                query.value("value").toDouble(),
                query.value("status").toString(),
                description
            );
              emit responseReady("The non-compete clause has been added to the contract.");
            return;
        }
    }
    emit errorOccurred("Contract not found.");
}

void ChatbotManager::createContract(const QJsonObject &jsonObj)
{
    QString clientName = jsonObj["client_name"].toString();
    QString startDateStr = jsonObj["start_date"].toString();
    QString endDateStr = jsonObj["end_date"].toString();
    double value = jsonObj["value"].toDouble();
    QString status = jsonObj["status"].toString();
    QString description = jsonObj["description"].toString();
    
    qDebug() << "Creating contract for client:" << clientName;
    
    // Generate a unique contract ID
    QString contractId = QString("CONT%1").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    
    // Validate required fields
    if (clientName.isEmpty()) {
        emit responseReady("❌ Error: Client name is required to create a contract.\n"
                          "Example: 'create contract for ABC Corporation'");
        return;
    }
    
    QDate startDate = QDate::fromString(startDateStr, "yyyy-MM-dd");
    QDate endDate = QDate::fromString(endDateStr, "yyyy-MM-dd");
    
    if (!startDate.isValid()) {
        startDate = QDate::currentDate();
        qDebug() << "Using current date as start date:" << startDate;
    }
    if (!endDate.isValid()) {
        endDate = startDate.addYears(1);
        qDebug() << "Using default end date (1 year from start):" << endDate;
    }
    
    // Validate date logic
    if (endDate <= startDate) {
        emit responseReady("❌ Error: End date must be after start date.\n"
                          "Please specify valid dates (format: YYYY-MM-DD)");
        return;
    }
    
    if (status.isEmpty()) {
        status = "Draft"; // Start as Draft by default
    }
    if (description.isEmpty()) {
        description = QString("Service contract for %1").arg(clientName);
    }
    if (value < 0) {
        value = 0; // Default to 0 if negative
    }
    
    bool success = m_dbManager->addContract(contractId, clientName, startDate, endDate, value, status, description);
      if (success) {
        emit responseReady(QString("✅ Contract created successfully!\n\n"
                                  "📋 Contract ID: %1\n"
                                  "👤 Client: %2\n"
                                  "💰 Value: $%3\n"
                                  "📅 Start Date: %4\n"
                                  "📅 End Date: %5\n"
                                  "📊 Status: %6\n"
                                  "📝 Description: %7")
                          .arg(contractId, clientName)
                          .arg(value, 0, 'f', 2)
                          .arg(startDate.toString("yyyy-MM-dd"))
                          .arg(endDate.toString("yyyy-MM-dd"))
                          .arg(status, description));
        emit databaseChanged(); // Notify that database was modified
    } else {
        emit responseReady("❌ Error: Failed to create the contract. Please check the database connection.");
    }
}

void ChatbotManager::updateContractField(const QJsonObject &jsonObj, const QString &contractId)
{
    QString field = jsonObj["field"].toString();
    QString value = jsonObj["value"].toString();
    
    if (field.isEmpty() || value.isEmpty()) {
        emit responseReady("Error: Both field and value are required for contract updates.");
        return;
    }
    
    // Determine how to find the contract
    QString targetContractId;
    
    // Check if contract_id is specified directly in the JSON
    if (jsonObj.contains("contract_id")) {
        targetContractId = jsonObj["contract_id"].toString();
    }
    // Check if we need to search by another field
    else if (jsonObj.contains("search_by") && jsonObj.contains("search_value")) {
        QString searchBy = jsonObj["search_by"].toString();
        QString searchValue = jsonObj["search_value"].toString();
        
        // Find contract by the specified search criteria
        QSqlQuery query = m_dbManager->getAllContracts();
        while (query.next()) {
            if (searchBy.toLower() == "client_name" || searchBy.toLower() == "client") {
                if (query.value("client_name").toString().toLower() == searchValue.toLower()) {
                    targetContractId = query.value("contract_id").toString();
                    break;
                }
            }
        }
        
        if (targetContractId.isEmpty()) {
            emit responseReady(QString("Error: No contract found with %1 = '%2'").arg(searchBy, searchValue));
            return;
        }
    }
    // Use the contractId parameter if provided
    else if (!contractId.isEmpty()) {
        targetContractId = contractId;
    }
    else {
        emit responseReady("Error: No contract specified for update. Please specify contract ID or search criteria.");
        return;
    }
    
    // Get current contract data
    QSqlQuery query = m_dbManager->getAllContracts();
    bool contractFound = false;
    QString clientName, status, description;
    QDate startDate, endDate;
    double contractValue = 0.0;
    
    while (query.next()) {
        if (query.value("contract_id").toString() == targetContractId) {
            contractFound = true;
            clientName = query.value("client_name").toString();
            startDate = query.value("start_date").toDate();
            endDate = query.value("end_date").toDate();
            contractValue = query.value("value").toDouble();
            status = query.value("status").toString();
            description = query.value("description").toString();
            break;
        }
    }
    
    if (!contractFound) {
        emit responseReady(QString("Error: Contract with ID '%1' not found.").arg(targetContractId));
        return;
    }
    
    // Update the specified field
    if (field.toLower() == "client_name" || field.toLower() == "client") {
        clientName = value;
    } else if (field.toLower() == "status") {
        status = value;
    } else if (field.toLower() == "description") {
        description = value;
    } else if (field.toLower() == "value" || field.toLower() == "amount") {
        bool ok;
        contractValue = value.toDouble(&ok);
        if (!ok) {
            emit responseReady("Error: Invalid value format. Please provide a number.");
            return;
        }
    } else if (field.toLower() == "start_date") {
        startDate = QDate::fromString(value, "yyyy-MM-dd");
        if (!startDate.isValid()) {
            emit responseReady("Error: Invalid start date format. Please use YYYY-MM-DD.");
            return;
        }
    } else if (field.toLower() == "end_date") {
        endDate = QDate::fromString(value, "yyyy-MM-dd");
        if (!endDate.isValid()) {
            emit responseReady("Error: Invalid end date format. Please use YYYY-MM-DD.");
            return;
        }
    } else {
        emit responseReady(QString("Error: Unknown field '%1'. Available fields: client_name, status, description, value, start_date, end_date").arg(field));
        return;
    }
    
    bool success = m_dbManager->updateContract(targetContractId, clientName, startDate, endDate, contractValue, status, description);
      if (success) {
        emit responseReady(QString("Contract %1 updated successfully. %2 changed to: %3").arg(targetContractId, field, value));
        emit databaseChanged(); // Notify that database was modified
    } else {
        emit responseReady("Error: Failed to update the contract.");
    }
}

void ChatbotManager::deleteContract(const QJsonObject &jsonObj)
{
    QString contractId = jsonObj["contract_id"].toString();
    
    if (contractId.isEmpty()) {
        emit responseReady("Error: Contract ID is required for deletion.");
        return;
    }
    
    bool success = m_dbManager->deleteContract(contractId);
      if (success) {
        emit responseReady(QString("Contract %1 has been deleted successfully.").arg(contractId));
        emit databaseChanged(); // Notify that database was modified
    } else {
        emit responseReady(QString("Error: Failed to delete contract %1. Please check if it exists.").arg(contractId));
    }
}

void ChatbotManager::searchContracts(const QJsonObject &jsonObj)
{
    QString searchTerm = jsonObj["search_term"].toString();
    QString status = jsonObj["status"].toString();
    
    qDebug() << "Searching contracts with term:" << searchTerm << "and status:" << status;
    
    QSqlQuery query;
    if (status.isEmpty() && searchTerm.isEmpty()) {
        query = m_dbManager->getAllContracts();
        qDebug() << "Getting all contracts";
    } else {
        query = m_dbManager->getFilteredContracts(searchTerm, status, QDate(), QDate(), 0, 999999999);
        qDebug() << "Using filtered search";
    }
    
    QStringList results;
    int count = 0;
    double totalValue = 0.0;
    
    while (query.next() && count < 15) { // Increased limit for better visibility
        QString contractId = query.value("contract_id").toString();
        QString clientName = query.value("client_name").toString();
        double value = query.value("value").toDouble();
        QString contractStatus = query.value("status").toString();
        QDate startDate = query.value("start_date").toDate();
        QDate endDate = query.value("end_date").toDate();
        QString description = query.value("description").toString();
        
        totalValue += value;
        
        QString contractInfo = QString("📋 Contract %1\n"
                                     "   👤 Client: %2\n"
                                     "   💰 Value: $%3\n"
                                     "   📊 Status: %4\n"
                                     "   📅 Period: %5 to %6")
                              .arg(contractId, clientName)
                              .arg(value, 0, 'f', 2)
                              .arg(contractStatus)
                              .arg(startDate.toString("yyyy-MM-dd"))
                              .arg(endDate.toString("yyyy-MM-dd"));
        
        if (!description.isEmpty()) {
            contractInfo += QString("\n   📝 Description: %1").arg(description.left(50) + (description.length() > 50 ? "..." : ""));
        }
        
        results << contractInfo;
        count++;
    }
    
    if (results.isEmpty()) {
        QString criteriaMsg = "";
        if (!searchTerm.isEmpty() && !status.isEmpty()) {
            criteriaMsg = QString(" matching term '%1' and status '%2'").arg(searchTerm, status);
        } else if (!searchTerm.isEmpty()) {
            criteriaMsg = QString(" matching term '%1'").arg(searchTerm);
        } else if (!status.isEmpty()) {
            criteriaMsg = QString(" with status '%1'").arg(status);
        }
        emit responseReady(QString("No contracts found%1.").arg(criteriaMsg));
    } else {
        QString header = QString("📋 Found %1 contract(s)").arg(count);
        if (totalValue > 0) {
            header += QString(" (Total Value: $%1)").arg(totalValue, 0, 'f', 2);
        }
        
        QString response = header + ":\n\n" + results.join("\n\n");
        
        if (count == 15) {
            response += "\n\n(Showing first 15 results)";
        }
        emit responseReady(response);
    }
}

void ChatbotManager::getContractStatistics()
{
    double totalValue = m_dbManager->getTotalContractValue();
    int activeCount = m_dbManager->getContractCountByStatus("Active");
    int completedCount = m_dbManager->getContractCountByStatus("Completed");
    int pendingCount = m_dbManager->getContractCountByStatus("Pending");
    
    QMap<QString, int> statusDistribution = m_dbManager->getContractStatusDistribution();
    
    QString statsResponse = QString(
        "📊 Contract Statistics:\n\n"
        "💰 Total Contract Value: $%1\n"
        "📋 Active Contracts: %2\n"
        "✅ Completed Contracts: %3\n"
        "⏳ Pending Contracts: %4\n\n"
        "Status Distribution:\n"
    ).arg(totalValue, 0, 'f', 2).arg(activeCount).arg(completedCount).arg(pendingCount);
    
    for (auto it = statusDistribution.begin(); it != statusDistribution.end(); ++it) {
        statsResponse += QString("• %1: %2 contracts\n").arg(it.key()).arg(it.value());
    }
    
    emit responseReady(statsResponse);
}

void ChatbotManager::getExpiringContracts(const QJsonObject &jsonObj)
{
    int days = jsonObj["days"].toInt();
    if (days <= 0) {
        days = 30; // Default to 30 days
    }
    
    QSqlQuery query = m_dbManager->getExpiringContracts(days);
    QStringList expiringContracts;
    
    while (query.next()) {
        QString contractInfo = QString("ID: %1 | Client: %2 | Expires: %3 | Value: $%4")
                              .arg(query.value("contract_id").toString())
                              .arg(query.value("client_name").toString())
                              .arg(query.value("end_date").toDate().toString("yyyy-MM-dd"))
                              .arg(query.value("value").toDouble(), 0, 'f', 2);
        expiringContracts << contractInfo;
    }
    
    if (expiringContracts.isEmpty()) {
        emit responseReady(QString("No contracts expiring in the next %1 days.").arg(days));
    } else {
        QString response = QString("⚠️ Contracts expiring in the next %1 days:\n\n%2")
                          .arg(days)
                          .arg(expiringContracts.join("\n"));
        emit responseReady(response);
    }
}

void ChatbotManager::parseAndExecuteCommand(const QString &apiResponse, const QString &contractId)
{
    qDebug() << "Attempting to parse LLM response. Full response received:" << apiResponse;

    QJsonDocument jsonDoc;
    QString stringToParse = apiResponse.trimmed(); // Initially, try to parse the whole response

    // If the response is clearly not JSON (doesn't start with { or [), treat it as a conversational response
    if (!stringToParse.startsWith("{") && !stringToParse.startsWith("[")) {
        qDebug() << "Response doesn't look like JSON, treating as conversational response";
        emit responseReady(apiResponse);
        return;
    }

    // Attempt 1: Try to parse the whole apiResponse directly
    jsonDoc = QJsonDocument::fromJson(stringToParse.toUtf8());

    if (!jsonDoc.isObject()) { // If direct parsing failed
        qDebug() << "Direct parsing of entire API response failed. Trying to extract from ```json ... ```";
        // Attempt 2: Try to extract from ```json ... ``` (captures content between the fences)
        QRegularExpression re("```json\\s*\\n([\\s\\S]*?)\\n\\s*```");
        QRegularExpressionMatch match = re.match(apiResponse); // Match against original full response

        if (match.hasMatch()) {
            stringToParse = match.captured(1).trimmed(); // Get the content within the fences and trim whitespace
            qDebug() << "Extracted JSON string from fences:" << stringToParse;
            jsonDoc = QJsonDocument::fromJson(stringToParse.toUtf8());
        } else {
            qDebug() << "Could not find ```json ... ``` block. Trying to find first '{' and last '}'.";
            // Attempt 3: Try to find the first '{' and last '}' in the original apiResponse
            int firstBrace = apiResponse.indexOf('{');
            int lastBrace = apiResponse.lastIndexOf('}');
            if (firstBrace != -1 && lastBrace != -1 && lastBrace > firstBrace) {
                stringToParse = apiResponse.mid(firstBrace, lastBrace - firstBrace + 1).trimmed();
                qDebug() << "Extracted JSON string by finding braces:" << stringToParse;
                jsonDoc = QJsonDocument::fromJson(stringToParse.toUtf8());
            } else {
                qDebug() << "Could not find any clear JSON block using braces either.";
                // If all parsing attempts fail, treat as conversational response
                qDebug() << "Treating as plain conversational response since JSON parsing failed";
                emit responseReady(apiResponse);
                return;
            }
        }
    }

    if (!jsonDoc.isObject()) {        
        qDebug() << "Error: LLM response could not be parsed as a valid JSON object after all attempts.";
        qDebug() << "Last string segment attempted for parsing:" << stringToParse;
        if (stringToParse != apiResponse) { // If we attempted a substring
            qDebug() << "Original full LLM response was:" << apiResponse;
        }
        // Instead of emitting an error, treat this as a conversational response
        qDebug() << "Falling back to treating response as conversational text";
        emit responseReady(apiResponse.isEmpty() ? "I'm here to help! How can I assist you?" : apiResponse);
        return;
    }

    qDebug() << "Successfully parsed JSON:" << jsonDoc.toJson(QJsonDocument::Compact);
    qDebug() << "parseAndExecuteCommand received contractId:" << contractId; // Log the contractId
    QJsonObject jsonObj = jsonDoc.object();    if (jsonObj.contains("type")) {
        QJsonValue typeValue = jsonObj["type"];
        qDebug() << "Parsed LLM command type:" << typeValue; // Log type
          // Check if type is null or the string "null"
        if (typeValue.isNull() || typeValue.toString() == "null" || typeValue.toString().isEmpty()) {
            emit responseReady("Hello! I'm your assistant for contract management. How can I help you?");
            return;
        }
        
        QString type = typeValue.toString();

        if (type == "general") {
            // Handle general conversation
            if (jsonObj.contains("response")) {
                QString response = jsonObj["response"].toString();
                if (!response.isEmpty()) {
                    emit responseReady(response);
                } else {
                    emit responseReady("I'm here to help! What would you like to know?");
                }
            } else {
                emit responseReady("I'm your assistant for contract management. How can I help you today?");
            }
            return;
        } else if (type == "payment_terms") {
            if (contractId.isEmpty()) {
                emit responseReady("To modify payment terms, please first select a contract in the application.");
                return;
            }
            
            QJsonValue value = jsonObj["value"];
            bool ok = false;
            int days = 0;

            if (value.isDouble()) {
                days = value.toInt();
                ok = true;
            } else if (value.isString()) {
                days = value.toString().toInt(&ok);
            }

            if (ok && days > 0) {
                updatePaymentTerms(contractId, days);
            } else {
                emit responseReady("Please specify a valid number of days for payment terms (e.g., 'modify payment terms to 30 days').");
            }
        } else if (type == "non_compete_clause") {
            if (contractId.isEmpty()) {
                emit responseReady("To add a non-compete clause, please first select a contract in the application.");
                return;
            }            addNonCompeteClause(contractId);
        } else if (type == "create_contract") {
            createContract(jsonObj);
        } else if (type == "update_contract") {
            updateContractField(jsonObj, contractId);
        } else if (type == "delete_contract") {
            deleteContract(jsonObj);
        } else if (type == "search_contracts") {
            searchContracts(jsonObj);
        } else if (type == "get_contract_stats") {
            getContractStatistics();
        } else if (type == "expiring_contracts") {
            getExpiringContracts(jsonObj);
        } else {
            // Handle unknown types conversationally
            emit responseReady("I'm not sure I understand that request. I can help you with:\n"
                              "• Contract creation (e.g., 'create a contract for ABC Corp')\n"
                              "• Contract updates (e.g., 'update contract status to completed')\n"
                              "• Contract deletion (e.g., 'delete contract CONT001')\n"
                              "• Contract search (e.g., 'show me all active contracts')\n"
                              "• Statistics (e.g., 'get contract statistics')\n"
                              "• Expiring contracts (e.g., 'contracts expiring in 30 days')\n"
                              "• Payment terms (e.g., 'modify terms to 30 days')\n"
                              "• Non-compete clauses (e.g., 'add a non-compete clause')\n"
                              "What would you like to do?");
        }
    } else {
        emit responseReady("Hello! I'm your assistant for contract management. How can I help you today?");
    }
}