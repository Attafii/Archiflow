#ifndef DATABASESERVICE_H
#define DATABASESERVICE_H

#include <QObject>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlRecord>
#include "../features/materials/materialmodel.h"

class DatabaseManager;
class MaterialModel;

/**
 * @brief High-level database service for AI assistant integration
 * 
 * This class provides a simplified interface for database operations
 * that can be easily used by the AI assistant to query and manipulate data.
 */
class DatabaseService : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseService(QObject *parent = nullptr);
    ~DatabaseService();
    
    // Initialization
    void setDatabaseManager(DatabaseManager *dbManager);
    void setMaterialModel(MaterialModel *materialModel);
    
    // Material operations
    QJsonArray getAllMaterials();
    QJsonObject getMaterialById(int id);
    QJsonArray searchMaterials(const QString &query);
    QJsonArray getMaterialsByCategory(const QString &category);
    QJsonArray getLowStockMaterials();
    QJsonArray getMaterialsByStatus(const QString &status);
    
    bool addMaterial(const QJsonObject &materialData);
    bool updateMaterial(int id, const QJsonObject &materialData);
    bool deleteMaterial(int id);
    
    // Bulk operations
    bool addMultipleMaterials(const QJsonArray &materialsData);
    bool updateMultipleMaterials(const QJsonArray &materialsData);
    
    // Statistics and analytics
    QJsonObject getDashboardStats();
    QJsonObject getInventoryAnalysis();
    QJsonArray getCategoryStats();
    QJsonObject getPriceAnalysis();
    QJsonArray getReorderAlerts();
      // Database management
    QJsonObject getDatabaseInfo();
    bool backupDatabase(const QString &filePath);
    bool restoreDatabase(const QString &filePath);
    bool resetDatabase();
    bool refreshDefaultMaterials();
    
    // Query execution (for advanced operations)
    QJsonArray executeCustomQuery(const QString &query, const QVariantList &params = QVariantList());
    bool executeCustomCommand(const QString &command, const QVariantList &params = QVariantList());
    
    // Data validation
    bool validateMaterialData(const QJsonObject &materialData, QString &errorMessage);
    QStringList getValidCategories();
    QStringList getValidUnits();
    QStringList getValidStatuses();
    
    // Import/Export
    QJsonArray exportMaterialsToJson();
    bool importMaterialsFromJson(const QJsonArray &materialsData);
    QString exportMaterialsToCsv();
    bool importMaterialsFromCsv(const QString &csvData);

signals:
    void operationCompleted(const QString &operation, bool success, const QString &message);
    void dataChanged();

private:
    Material jsonToMaterial(const QJsonObject &json);
    QJsonObject materialToJson(const Material &material);
    QString sanitizeQuery(const QString &query);
    bool isValidSqlQuery(const QString &query);
    
    DatabaseManager *m_dbManager;
    MaterialModel *m_materialModel;
    
    // Helper function to convert QSqlRecord to QJsonObject
    QJsonObject recordToJson(const QSqlRecord &record);
};

#endif // DATABASESERVICE_H
