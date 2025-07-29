#include "environmentloader.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

QHash<QString, QString> EnvironmentLoader::s_envVars;
bool EnvironmentLoader::s_loaded = false;

bool EnvironmentLoader::loadFromFile(const QString& envFilePath)
{
    QString filePath = envFilePath;
    if (filePath.isEmpty()) {
        // Default to .env in application directory
        filePath = QCoreApplication::applicationDirPath() + "/.env";
        
        // If not found, try the source directory (for development)
        if (!QFile::exists(filePath)) {
            QString sourceDir = QDir::currentPath();
            filePath = sourceDir + "/.env";
        }
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open .env file:" << filePath;
        return false;
    }
    
    QTextStream in(&file);
    QRegularExpression envRegex("^\\s*([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*(.*)\\s*$");
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith("#")) {
            continue;
        }
        
        QRegularExpressionMatch match = envRegex.match(line);
        if (match.hasMatch()) {
            QString key = match.captured(1);
            QString value = match.captured(2);
            
            // Remove quotes if present
            if ((value.startsWith('"') && value.endsWith('"')) ||
                (value.startsWith('\'') && value.endsWith('\''))) {
                value = value.mid(1, value.length() - 2);
            }
            
            s_envVars[key] = value;
            qDebug() << "Loaded env var:" << key << "=" << (key.contains("KEY") ? "[HIDDEN]" : value);
        }
    }
    
    s_loaded = true;
    return true;
}

QString EnvironmentLoader::getEnv(const QString& key, const QString& defaultValue)
{
    if (!s_loaded) {
        loadFromFile();
    }
    
    // First check loaded .env variables
    if (s_envVars.contains(key)) {
        return s_envVars[key];
    }
    
    // Then check system environment variables
    QByteArray sysEnv = qgetenv(key.toLocal8Bit());
    if (!sysEnv.isEmpty()) {
        return QString::fromLocal8Bit(sysEnv);
    }
    
    return defaultValue;
}
