#ifndef ICONTRACTCHATBOT_H
#define ICONTRACTCHATBOT_H

#include <QObject>
#include <QString>
#include <QStringList>

class Contract;

/**
 * @brief Interface for AI-powered contract chatbot functionality
 * 
 * This interface defines the contract for AI chatbot services that can
 * help users with contract-related queries, analysis, and recommendations.
 */
class IContractChatbot
{
public:
    virtual ~IContractChatbot() = default;

    // Chat operations
    virtual QString processQuery(const QString &query) = 0;
    virtual QStringList getSuggestions(const QString &partialQuery) = 0;
    virtual QString analyzeContract(const Contract *contract) = 0;
    virtual QString compareContracts(const QList<Contract*> &contracts) = 0;
    
    // Analysis features
    virtual QString identifyRisks(const Contract *contract) = 0;
    virtual QStringList recommendImprovements(const Contract *contract) = 0;
    virtual QString generateSummary(const Contract *contract) = 0;
    virtual QString extractKeyTerms(const Contract *contract) = 0;
    
    // Query types
    virtual QString answerContractQuestion(const QString &question, const Contract *contract) = 0;
    virtual QString provideContractGuidance(const QString &topic) = 0;
    virtual QStringList searchSimilarContracts(const Contract *contract) = 0;
    
    // Settings and configuration
    virtual void setAnalysisDepth(int depth) = 0; // 1-5 scale
    virtual void setLanguageModel(const QString &model) = 0;
    virtual void enableFeature(const QString &feature, bool enabled) = 0;
    
    // Status and capabilities
    virtual bool isAvailable() const = 0;
    virtual QStringList getAvailableFeatures() const = 0;
    virtual QString getLastError() const = 0;
};

Q_DECLARE_INTERFACE(IContractChatbot, "com.archiflow.IContractChatbot/1.0")

#endif // ICONTRACTCHATBOT_H
