#ifndef CHATBOTMANAGER_H
#define CHATBOTMANAGER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "databasemanager.h"

class ChatbotManager : public QObject
{
    Q_OBJECT

public:
    explicit ChatbotManager(DatabaseManager* dbManager, QObject *parent = nullptr);
    ~ChatbotManager();

    // Méthode principale pour traiter les requêtes utilisateur
    void processUserRequest(const QString &userInput, const QString &contractId);

signals:
    void responseReady(const QString &response);
    void errorOccurred(const QString &error);

private slots:
    void handleApiResponse(QNetworkReply *reply);

private:
    DatabaseManager* m_dbManager;
    QNetworkAccessManager* m_networkManager;
    QString m_apiKey;
    int m_retryCount;
    static const int MAX_RETRIES = 3;
    static const int INITIAL_RETRY_DELAY = 1000; // en millisecondes

    void retryApiRequest(const QString &prompt, int attempt, const QString &contractId);
    void handleAuthenticationError();
    void scheduleRetry(const QString &prompt, int attempt, const QString &contractId);

    // Méthodes privées pour le traitement des requêtes
    void sendApiRequest(const QString &prompt, const QString &contractId);
    void updatePaymentTerms(const QString &contractId, int days);
    void addNonCompeteClause(const QString &contractId);
    void parseAndExecuteCommand(const QString &apiResponse, const QString &contractId);
};

#endif // CHATBOTMANAGER_H