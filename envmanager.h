#ifndef ENVMANAGER_H
#define ENVMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>

class EnvManager : public QObject
{
    Q_OBJECT

public:
    static EnvManager& instance();

    // Get a value from the environment variables
    QString value(const QString& key, const QString& defaultValue = QString()) const;

    // Check if a key exists
    bool hasKey(const QString& key) const;

    // Reload environment variables from .env file
    bool reload();

private:
    explicit EnvManager(QObject *parent = nullptr);
    ~EnvManager() = default;

    // Make this class a singleton
    EnvManager(const EnvManager&) = delete;
    EnvManager& operator=(const EnvManager&) = delete;

    // Load environment variables from .env file
    bool loadEnvFile();

    QMap<QString, QString> m_variables;
};

#endif // ENVMANAGER_H