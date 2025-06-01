#ifndef GROQAIAPI_H
#define GROQAIAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QUrl>
#include <QNetworkRequest>
#include <QTimer>

class GroqAIAPI : public QObject
{
    Q_OBJECT

public:
    explicit GroqAIAPI(QObject *parent = nullptr);
    ~GroqAIAPI();

    // Set the API key
    void setApiKey(const QString &apiKey);
    
    // Get the API key
    QString apiKey() const;
    
    // Request cost estimation based on project details
    void requestCostEstimation(const QString &projectType, const QString &serviceArea, const QString &description, const QString &location = QString());

signals:
    // Signal emitted when cost estimation is received
    void costEstimationReceived(double cost, const QString &details);
    
    // Signal emitted when an error occurs
    void errorOccurred(const QString &errorMessage);

private slots:
    // Handle the network reply
    void handleNetworkReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_apiKey;
    
    // Create the request for cost estimation
    QNetworkRequest createRequest(const QUrl &url);
    
    // Create the JSON payload for the request
    QJsonDocument createRequestPayload(const QString &projectType, const QString &serviceArea, const QString &description, const QString &location = QString());
};

#endif // GROQAIAPI_H