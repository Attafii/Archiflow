#ifndef GROQCONTRACTCHATBOT_H
#define GROQCONTRACTCHATBOT_H

#include "../../interfaces/icontractchatbot.h"
#include "../materials/groqclient.h"
#include <QObject>

class Contract;

/**
 * @brief GROQ-powered implementation of contract chatbot
 * 
 * This class provides AI-powered contract assistance using the GROQ API,
 * implementing the IContractChatbot interface for seamless integration.
 */
class GroqContractChatbot : public QObject, public IContractChatbot
{
    Q_OBJECT
    Q_INTERFACES(IContractChatbot)

public:
    explicit GroqContractChatbot(GroqClient *groqClient, QObject *parent = nullptr);
    ~GroqContractChatbot();

    // IContractChatbot interface implementation
    QString processQuery(const QString &query) override;
    QStringList getSuggestions(const QString &partialQuery) override;
    QString analyzeContract(const Contract *contract) override;
    QString compareContracts(const QList<Contract*> &contracts) override;
    
    // Analysis features
    QString identifyRisks(const Contract *contract) override;
    QStringList recommendImprovements(const Contract *contract) override;
    QString generateSummary(const Contract *contract) override;
    QString extractKeyTerms(const Contract *contract) override;
    
    // Query types
    QString answerContractQuestion(const QString &question, const Contract *contract) override;
    QString provideContractGuidance(const QString &topic) override;
    QStringList searchSimilarContracts(const Contract *contract) override;
    
    // Settings and configuration
    void setAnalysisDepth(int depth) override;
    void setLanguageModel(const QString &model) override;
    void enableFeature(const QString &feature, bool enabled) override;
    
    // Status and capabilities
    bool isAvailable() const override;
    QStringList getAvailableFeatures() const override;
    QString getLastError() const override;

    // Additional GROQ-specific methods
    void setGroqClient(GroqClient *client);
    GroqClient* groqClient() const { return m_groqClient; }

signals:
    void queryProcessed(const QString &response);
    void errorOccurred(const QString &error);

private slots:
    void onGroqMessageReceived(const QString &message, const QString &messageId);
    void onGroqErrorOccurred(const QString &error, int code);

private:
    QString formatContractForAnalysis(const Contract *contract);
    QString createSystemPromptForContracts();
    QString createAnalysisPrompt(const QString &analysisType, const Contract *contract);
    QString createComparisonPrompt(const QList<Contract*> &contracts);
    QString processAsyncQuery(const QString &prompt);
    
    GroqClient *m_groqClient;
    QString m_lastError;
    QString m_lastResponse;
    int m_analysisDepth;
    QString m_languageModel;
    QMap<QString, bool> m_enabledFeatures;
    
    // Async operation support
    bool m_waitingForResponse;
    QEventLoop *m_eventLoop;
    QTimer *m_timeoutTimer;
    
    static const QString CONTRACT_SYSTEM_PROMPT;
    static const QStringList AVAILABLE_FEATURES;
    static const int DEFAULT_TIMEOUT_MS;
};

#endif // GROQCONTRACTCHATBOT_H
