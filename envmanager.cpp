#include "envmanager.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>

EnvManager::EnvManager(QObject *parent)
    : QObject(parent)
{
    loadEnvFile();
}

EnvManager& EnvManager::instance()
{
    static EnvManager instance;
    return instance;
}

QString EnvManager::value(const QString& key, const QString& defaultValue) const
{
    return m_variables.value(key, defaultValue);
}

bool EnvManager::hasKey(const QString& key) const
{
    return m_variables.contains(key);
}

bool EnvManager::reload()
{
    m_variables.clear();
    return loadEnvFile();
}

bool EnvManager::loadEnvFile()
{
    // Try to open the .env file in the application directory
    QFile file(QDir::currentPath() + "/.env");
    
    if (!file.exists()) {
        qDebug() << "EnvManager: .env file not found at" << file.fileName();
        
        // Try to find the .env file in the parent directory
        QDir parentDir = QDir::current();
        parentDir.cdUp();
        QFile parentFile(parentDir.absolutePath() + "/.env");
        
        if (parentFile.exists()) {
            qDebug() << "EnvManager: Found .env file in parent directory:" << parentFile.fileName();
            file.setFileName(parentFile.fileName());
        } else {
            qDebug() << "EnvManager: .env file not found in parent directory either:" << parentFile.fileName();
            return false;
        }
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "EnvManager: Failed to open .env file:" << file.errorString();
        return false;
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }
        
        // Parse KEY=VALUE format
        int separatorPos = line.indexOf('=');
        if (separatorPos > 0) {
            QString key = line.left(separatorPos).trimmed();
            QString value = line.mid(separatorPos + 1).trimmed();

            // Remove quotes if present
            if ((value.startsWith('"') && value.endsWith('"')) ||
                (value.startsWith('\'') && value.endsWith('\''))) {
                value = value.mid(1, value.length() - 2);
            }

            m_variables.insert(key, value);
        }
    }
    
    file.close();
    qDebug() << "EnvManager: Loaded" << m_variables.size() << "environment variables";
    return true;
}
