#ifndef ENVIRONMENTLOADER_H
#define ENVIRONMENTLOADER_H

#include <QString>
#include <QHash>

/**
 * @brief Simple utility to load environment variables from .env file
 */
class EnvironmentLoader
{
public:
    /**
     * @brief Load environment variables from .env file in the application directory
     * @param envFilePath Path to .env file (defaults to .env in app directory)
     * @return true if file was loaded successfully
     */
    static bool loadFromFile(const QString& envFilePath = QString());
    
    /**
     * @brief Get environment variable value
     * @param key Environment variable name
     * @param defaultValue Default value if not found
     * @return Environment variable value or default
     */
    static QString getEnv(const QString& key, const QString& defaultValue = QString());

private:
    static QHash<QString, QString> s_envVars;
    static bool s_loaded;
};

#endif // ENVIRONMENTLOADER_H
