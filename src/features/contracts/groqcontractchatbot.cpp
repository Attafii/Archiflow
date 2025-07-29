#include "groqcontractchatbot.h"
#include "contract.h"
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>

const QString GroqContractChatbot::CONTRACT_SYSTEM_PROMPT = 
    "You are an AI assistant specialized in contract management and legal document analysis for architecture and construction projects. "
    "You help users with contract review, risk assessment, clause analysis, compliance checking, payment terms evaluation, "
    "and contract lifecycle management. Provide clear, professional, and legally-informed assistance while noting that "
    "users should consult with qualified legal professionals for important decisions. Focus on construction industry best practices.";

const QStringList GroqContractChatbot::AVAILABLE_FEATURES = {
    "Risk Analysis", "Contract Comparison", "Clause Extraction", "Summary Generation",
    "Payment Terms Analysis", "Compliance Check", "Contract Recommendations", "Key Terms Identification"
};

const int GroqContractChatbot::DEFAULT_TIMEOUT_MS = 30000;

GroqContractChatbot::GroqContractChatbot(GroqClient *groqClient, QObject *parent)
    : QObject(parent)
    , m_groqClient(groqClient)
    , m_analysisDepth(3)
    , m_languageModel("llama-3.3-70b-versatile")
    , m_waitingForResponse(false)
    , m_eventLoop(nullptr)
    , m_timeoutTimer(new QTimer(this))
{
    // Initialize enabled features
    for (const QString &feature : AVAILABLE_FEATURES) {
        m_enabledFeatures[feature] = true;
    }
    
    // Setup timeout timer
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, [this]() {
        m_lastError = "Request timed out";
        if (m_eventLoop && m_eventLoop->isRunning()) {
            m_eventLoop->quit();
        }
        m_waitingForResponse = false;
    });
    
    if (m_groqClient) {
        connect(m_groqClient, &GroqClient::messageReceived,
                this, &GroqContractChatbot::onGroqMessageReceived);
        connect(m_groqClient, &GroqClient::errorOccurred,
                this, &GroqContractChatbot::onGroqErrorOccurred);
        
        // Set contract-specific system prompt
        m_groqClient->setSystemPrompt(CONTRACT_SYSTEM_PROMPT);
    }
}

GroqContractChatbot::~GroqContractChatbot()
{
    if (m_eventLoop && m_eventLoop->isRunning()) {
        m_eventLoop->quit();
    }
}

void GroqContractChatbot::setGroqClient(GroqClient *client)
{
    if (m_groqClient) {
        disconnect(m_groqClient, nullptr, this, nullptr);
    }
    
    m_groqClient = client;
    
    if (m_groqClient) {
        connect(m_groqClient, &GroqClient::messageReceived,
                this, &GroqContractChatbot::onGroqMessageReceived);
        connect(m_groqClient, &GroqClient::errorOccurred,
                this, &GroqContractChatbot::onGroqErrorOccurred);
        
        m_groqClient->setSystemPrompt(CONTRACT_SYSTEM_PROMPT);
    }
}

QString GroqContractChatbot::processQuery(const QString &query)
{
    if (!m_groqClient || !isAvailable()) {
        m_lastError = "GroqClient not available";
        return QString();
    }
    
    return processAsyncQuery(query);
}

QString GroqContractChatbot::processAsyncQuery(const QString &prompt)
{
    if (m_waitingForResponse) {
        m_lastError = "Another query is already in progress";
        return QString();
    }
    
    m_waitingForResponse = true;
    m_lastResponse.clear();
    m_lastError.clear();
    
    // Send the query
    m_groqClient->sendMessage(prompt);
    
    // Wait for response with timeout
    QEventLoop eventLoop;
    m_eventLoop = &eventLoop;
    m_timeoutTimer->start(DEFAULT_TIMEOUT_MS);
    
    eventLoop.exec();
    
    m_eventLoop = nullptr;
    m_timeoutTimer->stop();
    m_waitingForResponse = false;
    
    return m_lastResponse;
}

QStringList GroqContractChatbot::getSuggestions(const QString &partialQuery)
{
    if (!isAvailable()) {
        return QStringList();
    }
    
    QString prompt = QString(
        "Based on the partial contract management query: '%1'\n"
        "Provide 3-5 relevant suggestions for completing this query. "
        "Focus on common contract management tasks, analysis types, or questions. "
        "Return only the suggestions, one per line, without numbers or bullets."
    ).arg(partialQuery);
    
    QString response = processAsyncQuery(prompt);
    return response.split('\n', Qt::SkipEmptyParts);
}

QString GroqContractChatbot::analyzeContract(const Contract *contract)
{
    if (!contract || !isAvailable()) {
        m_lastError = "Invalid contract or service unavailable";
        return QString();
    }
    
    QString contractData = formatContractForAnalysis(contract);
    QString prompt = createAnalysisPrompt("comprehensive_analysis", contract);
    
    return processAsyncQuery(prompt + "\n\nContract Data:\n" + contractData);
}

QString GroqContractChatbot::compareContracts(const QList<Contract*> &contracts)
{
    if (contracts.isEmpty() || !isAvailable()) {
        m_lastError = "No contracts provided or service unavailable";
        return QString();
    }
    
    QString prompt = createComparisonPrompt(contracts);
    return processAsyncQuery(prompt);
}

QString GroqContractChatbot::identifyRisks(const Contract *contract)
{
    if (!contract || !isAvailable()) {
        m_lastError = "Invalid contract or service unavailable";
        return QString();
    }
    
    QString contractData = formatContractForAnalysis(contract);
    QString prompt = createAnalysisPrompt("risk_analysis", contract);
    
    return processAsyncQuery(prompt + "\n\nContract Data:\n" + contractData);
}

QStringList GroqContractChatbot::recommendImprovements(const Contract *contract)
{
    if (!contract || !isAvailable()) {
        return QStringList();
    }
    
    QString contractData = formatContractForAnalysis(contract);
    QString prompt = createAnalysisPrompt("improvement_recommendations", contract);
    
    QString response = processAsyncQuery(prompt + "\n\nContract Data:\n" + contractData);
    
    // Parse response into list of recommendations
    QStringList recommendations;
    QStringList lines = response.split('\n', Qt::SkipEmptyParts);
    
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith("•") || trimmed.startsWith("-") || trimmed.startsWith("*")) {
            recommendations.append(trimmed.mid(1).trimmed());
        } else if (!trimmed.isEmpty() && (trimmed.contains("recommend") || trimmed.contains("improve") || trimmed.contains("consider"))) {
            recommendations.append(trimmed);
        }
    }
    
    return recommendations;
}

QString GroqContractChatbot::generateSummary(const Contract *contract)
{
    if (!contract || !isAvailable()) {
        m_lastError = "Invalid contract or service unavailable";
        return QString();
    }
    
    QString contractData = formatContractForAnalysis(contract);
    QString prompt = createAnalysisPrompt("summary", contract);
    
    return processAsyncQuery(prompt + "\n\nContract Data:\n" + contractData);
}

QString GroqContractChatbot::extractKeyTerms(const Contract *contract)
{
    if (!contract || !isAvailable()) {
        m_lastError = "Invalid contract or service unavailable";
        return QString();
    }
    
    QString contractData = formatContractForAnalysis(contract);
    QString prompt = createAnalysisPrompt("key_terms", contract);
    
    return processAsyncQuery(prompt + "\n\nContract Data:\n" + contractData);
}

QString GroqContractChatbot::answerContractQuestion(const QString &question, const Contract *contract)
{
    if (!isAvailable()) {
        m_lastError = "Service unavailable";
        return QString();
    }
    
    QString prompt = QString(
        "Answer this specific question about the contract: %1\n\n"
        "Please provide a clear, detailed answer based on the contract information provided."
    ).arg(question);
    
    if (contract) {
        QString contractData = formatContractForAnalysis(contract);
        prompt += "\n\nContract Data:\n" + contractData;
    }
    
    return processAsyncQuery(prompt);
}

QString GroqContractChatbot::provideContractGuidance(const QString &topic)
{
    if (!isAvailable()) {
        m_lastError = "Service unavailable";
        return QString();
    }
    
    QString prompt = QString(
        "Provide professional guidance on this contract management topic: %1\n\n"
        "Focus on construction industry best practices, legal considerations, and practical advice. "
        "Include relevant clauses, terms, and recommendations where appropriate."
    ).arg(topic);
    
    return processAsyncQuery(prompt);
}

QStringList GroqContractChatbot::searchSimilarContracts(const Contract *contract)
{
    // This would typically integrate with a database search
    // For now, return a placeholder implementation
    QStringList similar;
    if (contract && isAvailable()) {
        QString prompt = QString(
            "Based on this contract's characteristics (Client: %1, Value: %2, Type: Construction), "
            "what are the key criteria I should use to search for similar contracts in my database? "
            "Provide 3-5 specific search criteria."
        ).arg(contract->clientName()).arg(contract->value());
        
        QString response = processAsyncQuery(prompt);
        similar = response.split('\n', Qt::SkipEmptyParts);
    }
    return similar;
}

void GroqContractChatbot::setAnalysisDepth(int depth)
{
    m_analysisDepth = qBound(1, depth, 5);
}

void GroqContractChatbot::setLanguageModel(const QString &model)
{
    m_languageModel = model;
    if (m_groqClient) {
        GroqConfig config = m_groqClient->configuration();
        config.model = model;
        m_groqClient->setConfiguration(config);
    }
}

void GroqContractChatbot::enableFeature(const QString &feature, bool enabled)
{
    m_enabledFeatures[feature] = enabled;
}

bool GroqContractChatbot::isAvailable() const
{
    return m_groqClient && m_groqClient->isConnected() && !m_groqClient->isBusy();
}

QStringList GroqContractChatbot::getAvailableFeatures() const
{
    QStringList enabled;
    for (auto it = m_enabledFeatures.constBegin(); it != m_enabledFeatures.constEnd(); ++it) {
        if (it.value()) {
            enabled.append(it.key());
        }
    }
    return enabled;
}

QString GroqContractChatbot::getLastError() const
{
    return m_lastError;
}

void GroqContractChatbot::onGroqMessageReceived(const QString &message, const QString &messageId)
{
    Q_UNUSED(messageId)
    m_lastResponse = message;
    if (m_eventLoop && m_eventLoop->isRunning()) {
        m_eventLoop->quit();
    }
    emit queryProcessed(message);
}

void GroqContractChatbot::onGroqErrorOccurred(const QString &error, int code)
{
    Q_UNUSED(code)
    m_lastError = error;
    if (m_eventLoop && m_eventLoop->isRunning()) {
        m_eventLoop->quit();
    }
    emit errorOccurred(error);
}

QString GroqContractChatbot::formatContractForAnalysis(const Contract *contract)
{
    if (!contract) return QString();
    
    return QString(
        "Contract ID: %1\n"
        "Client Name: %2\n"
        "Start Date: %3\n"
        "End Date: %4\n"
        "Value: $%5\n"
        "Status: %6\n"        "Payment Terms: %7\n"
        "Description: %8\n"
    ).arg(contract->id())
     .arg(contract->clientName())
     .arg(contract->startDate().toString("yyyy-MM-dd"))
     .arg(contract->endDate().toString("yyyy-MM-dd"))
     .arg(contract->value(), 0, 'f', 2)
     .arg(contract->status())
     .arg(contract->paymentTermsString())
     .arg(contract->description());
}

QString GroqContractChatbot::createAnalysisPrompt(const QString &analysisType, const Contract *contract)
{
    Q_UNUSED(contract)
    
    QString basePrompt = "You are analyzing a construction contract. ";
    
    if (analysisType == "comprehensive_analysis") {
        basePrompt += "Provide a comprehensive analysis covering key terms, payment structure, timeline, "
                     "deliverables, risks, and compliance considerations. Structure your response with clear headings.";
    } else if (analysisType == "risk_analysis") {
        basePrompt += "Focus specifically on identifying potential risks including financial risks, timeline risks, "
                     "scope creep, payment delays, compliance issues, and force majeure considerations. "
                     "Prioritize risks by severity and likelihood.";
    } else if (analysisType == "improvement_recommendations") {
        basePrompt += "Provide specific recommendations for improving this contract. Focus on clauses that could be "
                     "strengthened, missing provisions, better payment terms, clearer deliverables, and risk mitigation. "
                     "Format as bullet points.";
    } else if (analysisType == "summary") {
        basePrompt += "Create a concise executive summary of this contract highlighting the key points: parties involved, "
                     "project scope, timeline, financial terms, and critical obligations.";
    } else if (analysisType == "key_terms") {
        basePrompt += "Extract and explain the most important terms and clauses from this contract. Focus on payment terms, "
                     "deliverables, deadlines, penalties, and special conditions.";
    }
    
    return basePrompt;
}

QString GroqContractChatbot::createComparisonPrompt(const QList<Contract*> &contracts)
{
    QString prompt = QString("Compare these %1 construction contracts and provide insights on:\n").arg(contracts.size());
    prompt += "• Value and payment terms differences\n";
    prompt += "• Timeline and duration variations\n";
    prompt += "• Risk profiles and mitigation strategies\n";
    prompt += "• Best practices observed\n";
    prompt += "• Recommendations for standardization\n\n";
    
    for (int i = 0; i < contracts.size(); ++i) {
        prompt += QString("Contract %1:\n").arg(i + 1);
        prompt += formatContractForAnalysis(contracts[i]) + "\n";
    }
    
    return prompt;
}
