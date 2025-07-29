#include "databaseservice.h"
#include "databasemanager.h"
#include "../features/materials/materialmodel.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QRegularExpression>
#include <QDebug>
#include <QDateTime>
#include <QFileInfo>

DatabaseService::DatabaseService(QObject *parent)
    : QObject(parent)
    , m_dbManager(nullptr)
    , m_materialModel(nullptr)
{
}

DatabaseService::~DatabaseService()
{
}

void DatabaseService::setDatabaseManager(DatabaseManager *dbManager)
{
    m_dbManager = dbManager;
}

void DatabaseService::setMaterialModel(MaterialModel *materialModel)
{
    m_materialModel = materialModel;
}

QJsonArray DatabaseService::getAllMaterials()
{
    QJsonArray result;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        return result;
    }
      QSqlQuery query = m_dbManager->executeQuery("SELECT * FROM materials ORDER BY name");
      while (query.next()) {
        result.append(recordToJson(query.record()));
    }
    
    return result;
}

QJsonObject DatabaseService::getMaterialById(int id)
{
    QJsonObject result;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        return result;
    }
    
    QSqlQuery query = m_dbManager->executeQuery("SELECT * FROM materials WHERE id = ?", {id});
      if (query.next()) {
        QSqlRecord record = query.record();
        
        for (int i = 0; i < record.count(); ++i) {
            QString fieldName = record.fieldName(i);
            QVariant value = record.value(i);
            
            if (value.metaType() == QMetaType::fromType<QDateTime>()) {
                result[fieldName] = value.toDateTime().toString(Qt::ISODate);
            } else {
                result[fieldName] = QJsonValue::fromVariant(value);
            }
        }
    }
    
    return result;
}

QJsonArray DatabaseService::searchMaterials(const QString &query)
{
    QJsonArray result;
    
    if (!m_dbManager || !m_dbManager->isConnected() || query.trimmed().isEmpty()) {
        return result;
    }
    
    QString searchQuery = QString(
        "SELECT * FROM materials WHERE "
        "name LIKE ? OR description LIKE ? OR category LIKE ? OR barcode LIKE ? "
        "ORDER BY name"
    );
    
    QString searchTerm = QString("%%1%").arg(query);
    QVariantList params = {searchTerm, searchTerm, searchTerm, searchTerm};
    
    QSqlQuery sqlQuery = m_dbManager->executeQuery(searchQuery, params);    
    while (sqlQuery.next()) {
        QJsonObject material;
        QSqlRecord record = sqlQuery.record();
        
        for (int i = 0; i < record.count(); ++i) {
            QString fieldName = record.fieldName(i);
            QVariant value = record.value(i);
            
            if (value.metaType() == QMetaType::fromType<QDateTime>()) {
                material[fieldName] = value.toDateTime().toString(Qt::ISODate);
            } else {
                material[fieldName] = QJsonValue::fromVariant(value);
            }
        }
        result.append(material);
    }
    
    return result;
}

QJsonArray DatabaseService::getMaterialsByCategory(const QString &category)
{
    QJsonArray result;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        return result;
    }
      QSqlQuery query = m_dbManager->executeQuery(
        "SELECT * FROM materials WHERE category = ? ORDER BY name", {category});
    
    while (query.next()) {
        QJsonObject material;
        QSqlRecord record = query.record();
        
        for (int i = 0; i < record.count(); ++i) {
            QString fieldName = record.fieldName(i);
            QVariant value = record.value(i);
            
            if (value.metaType() == QMetaType::fromType<QDateTime>()) {
                material[fieldName] = value.toDateTime().toString(Qt::ISODate);
            } else {
                material[fieldName] = QJsonValue::fromVariant(value);
            }
        }
        result.append(material);
    }
    
    return result;
}

QJsonArray DatabaseService::getLowStockMaterials()
{
    QJsonArray result;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        return result;
    }
      QSqlQuery query = m_dbManager->executeQuery(
        "SELECT * FROM materials WHERE quantity <= reorderPoint ORDER BY quantity ASC");
    
    while (query.next()) {
        QJsonObject material;
        QSqlRecord record = query.record();
        
        for (int i = 0; i < record.count(); ++i) {
            QString fieldName = record.fieldName(i);
            QVariant value = record.value(i);
            
            if (value.metaType() == QMetaType::fromType<QDateTime>()) {
                material[fieldName] = value.toDateTime().toString(Qt::ISODate);
            } else {
                material[fieldName] = QJsonValue::fromVariant(value);
            }
        }
        result.append(material);
    }
    
    return result;
}

QJsonArray DatabaseService::getMaterialsByStatus(const QString &status)
{
    QJsonArray result;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        return result;
    }
      QSqlQuery query = m_dbManager->executeQuery(
        "SELECT * FROM materials WHERE status = ? ORDER BY name", {status});
    
    while (query.next()) {
        QJsonObject material;
        QSqlRecord record = query.record();
        
        for (int i = 0; i < record.count(); ++i) {
            QString fieldName = record.fieldName(i);
            QVariant value = record.value(i);
            
            if (value.metaType() == QMetaType::fromType<QDateTime>()) {
                material[fieldName] = value.toDateTime().toString(Qt::ISODate);
            } else {
                material[fieldName] = QJsonValue::fromVariant(value);
            }
        }
        result.append(material);
    }
    
    return result;
}

bool DatabaseService::addMaterial(const QJsonObject &materialData)
{
    if (!m_materialModel) {
        return false;
    }
    
    QString errorMessage;
    if (!validateMaterialData(materialData, errorMessage)) {
        qDebug() << "Material validation failed:" << errorMessage;
        return false;
    }
    
    Material material = jsonToMaterial(materialData);
    bool success = m_materialModel->addMaterial(material);
    
    if (success) {
        emit operationCompleted("addMaterial", true, "Material added successfully");
        emit dataChanged();
    } else {
        emit operationCompleted("addMaterial", false, "Failed to add material");
    }
    
    return success;
}

bool DatabaseService::updateMaterial(int id, const QJsonObject &materialData)
{
    if (!m_materialModel) {
        return false;
    }
    
    QString errorMessage;
    if (!validateMaterialData(materialData, errorMessage)) {
        qDebug() << "Material validation failed:" << errorMessage;
        return false;
    }
    
    // Find the material row by ID
    for (int row = 0; row < m_materialModel->rowCount(); ++row) {
        Material existing = m_materialModel->getMaterial(row);
        if (existing.id == id) {
            Material updated = jsonToMaterial(materialData);
            updated.id = id; // Preserve the ID
            updated.updatedAt = QDateTime::currentDateTime();
            
            bool success = m_materialModel->updateMaterial(row, updated);
            
            if (success) {
                emit operationCompleted("updateMaterial", true, "Material updated successfully");
                emit dataChanged();
            } else {
                emit operationCompleted("updateMaterial", false, "Failed to update material");
            }
            
            return success;
        }
    }
    
    return false;
}

bool DatabaseService::deleteMaterial(int id)
{
    if (!m_materialModel) {
        return false;
    }
    
    // Find the material row by ID
    for (int row = 0; row < m_materialModel->rowCount(); ++row) {
        Material existing = m_materialModel->getMaterial(row);
        if (existing.id == id) {
            m_materialModel->removeMaterial(row);
            emit operationCompleted("deleteMaterial", true, "Material deleted successfully");
            emit dataChanged();
            return true;
        }
    }
    
    emit operationCompleted("deleteMaterial", false, "Material not found");
    return false;
}

QJsonObject DatabaseService::getDashboardStats()
{
    QJsonObject stats;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        return stats;
    }
    
    // Total materials
    QSqlQuery totalQuery = m_dbManager->executeQuery("SELECT COUNT(*) FROM materials");
    if (totalQuery.next()) {
        stats["totalMaterials"] = totalQuery.value(0).toInt();
    }
    
    // Active materials
    QSqlQuery activeQuery = m_dbManager->executeQuery("SELECT COUNT(*) FROM materials WHERE status = 'active'");
    if (activeQuery.next()) {
        stats["activeMaterials"] = activeQuery.value(0).toInt();
    }
    
    // Low stock count
    QSqlQuery lowStockQuery = m_dbManager->executeQuery("SELECT COUNT(*) FROM materials WHERE quantity <= reorderPoint");
    if (lowStockQuery.next()) {
        stats["lowStockCount"] = lowStockQuery.value(0).toInt();
    }
    
    // Total inventory value
    QSqlQuery valueQuery = m_dbManager->executeQuery("SELECT SUM(quantity * price) FROM materials WHERE status = 'active'");
    if (valueQuery.next()) {
        stats["totalValue"] = valueQuery.value(0).toDouble();
    }
    
    // Categories count
    QSqlQuery categoriesQuery = m_dbManager->executeQuery("SELECT COUNT(DISTINCT category) FROM materials");
    if (categoriesQuery.next()) {
        stats["categoriesCount"] = categoriesQuery.value(0).toInt();
    }
    
    return stats;
}

QJsonArray DatabaseService::getCategoryStats()
{
    QJsonArray result;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        return result;
    }
    
    QSqlQuery query = m_dbManager->executeQuery(
        "SELECT category, COUNT(*) as count, SUM(quantity * price) as value "
        "FROM materials GROUP BY category ORDER BY count DESC");
    
    while (query.next()) {
        QJsonObject categoryStats;
        categoryStats["category"] = query.value("category").toString();
        categoryStats["count"] = query.value("count").toInt();
        categoryStats["value"] = query.value("value").toDouble();
        result.append(categoryStats);
    }
    
    return result;
}

QJsonArray DatabaseService::getReorderAlerts()
{
    return getLowStockMaterials(); // Same as low stock materials
}

Material DatabaseService::jsonToMaterial(const QJsonObject &json)
{
    Material material;
    
    material.id = json["id"].toInt();
    material.name = json["name"].toString();
    material.description = json["description"].toString();
    material.category = json["category"].toString();
    material.quantity = json["quantity"].toInt();
    material.unit = json["unit"].toString();
    material.price = json["price"].toDouble();
    material.supplierId = json["supplierId"].toInt();
    material.barcode = json["barcode"].toString();
    material.location = json["location"].toString();
    material.minimumStock = json["minimumStock"].toInt();
    material.maximumStock = json["maximumStock"].toInt();
    material.reorderPoint = json["reorderPoint"].toInt();
    material.status = json["status"].toString();
    material.createdBy = json["createdBy"].toString();
    material.updatedBy = json["updatedBy"].toString();
    
    if (json.contains("createdAt") && !json["createdAt"].toString().isEmpty()) {
        material.createdAt = QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate);
    } else {
        material.createdAt = QDateTime::currentDateTime();
    }
    
    if (json.contains("updatedAt") && !json["updatedAt"].toString().isEmpty()) {
        material.updatedAt = QDateTime::fromString(json["updatedAt"].toString(), Qt::ISODate);
    } else {
        material.updatedAt = QDateTime::currentDateTime();
    }
    
    return material;
}

QJsonObject DatabaseService::materialToJson(const Material &material)
{
    QJsonObject json;
    
    json["id"] = material.id;
    json["name"] = material.name;
    json["description"] = material.description;
    json["category"] = material.category;
    json["quantity"] = material.quantity;
    json["unit"] = material.unit;
    json["price"] = material.price;
    json["supplierId"] = material.supplierId;
    json["barcode"] = material.barcode;
    json["location"] = material.location;
    json["minimumStock"] = material.minimumStock;
    json["maximumStock"] = material.maximumStock;
    json["reorderPoint"] = material.reorderPoint;
    json["status"] = material.status;
    json["createdBy"] = material.createdBy;
    json["updatedBy"] = material.updatedBy;
    json["createdAt"] = material.createdAt.toString(Qt::ISODate);
    json["updatedAt"] = material.updatedAt.toString(Qt::ISODate);
    
    return json;
}

bool DatabaseService::validateMaterialData(const QJsonObject &materialData, QString &errorMessage)
{
    // Check required fields
    if (!materialData.contains("name") || materialData["name"].toString().trimmed().isEmpty()) {
        errorMessage = "Material name is required";
        return false;
    }
    
    if (!materialData.contains("category") || materialData["category"].toString().trimmed().isEmpty()) {
        errorMessage = "Material category is required";
        return false;
    }
    
    if (materialData.contains("quantity") && materialData["quantity"].toInt() < 0) {
        errorMessage = "Quantity cannot be negative";
        return false;
    }
    
    if (materialData.contains("price") && materialData["price"].toDouble() < 0) {
        errorMessage = "Price cannot be negative";
        return false;
    }
    
    if (materialData.contains("minimumStock") && materialData["minimumStock"].toInt() < 0) {
        errorMessage = "Minimum stock cannot be negative";
        return false;
    }
    
    if (materialData.contains("maximumStock") && materialData["maximumStock"].toInt() < 0) {
        errorMessage = "Maximum stock cannot be negative";
        return false;
    }
    
    if (materialData.contains("reorderPoint") && materialData["reorderPoint"].toInt() < 0) {
        errorMessage = "Reorder point cannot be negative";
        return false;
    }
    
    // Validate status
    if (materialData.contains("status")) {
        QString status = materialData["status"].toString();
        QStringList validStatuses = {"active", "inactive", "discontinued"};
        if (!validStatuses.contains(status)) {
            errorMessage = QString("Invalid status: %1. Valid values are: %2")
                          .arg(status).arg(validStatuses.join(", "));
            return false;
        }
    }
    
    return true;
}

QStringList DatabaseService::getValidCategories()
{
    return {"Construction", "Electrical", "Plumbing", "HVAC", "Flooring", "Roofing", 
            "Insulation", "Hardware", "Tools", "Safety", "Concrete", "Steel", "Wood", "Other"};
}

QStringList DatabaseService::getValidUnits()
{
    return {"pcs", "kg", "m", "m²", "m³", "l", "box", "roll", "bag", "ton", "ft", "yd", "gal"};
}

QStringList DatabaseService::getValidStatuses()
{
    return {"active", "inactive", "discontinued"};
}

QJsonObject DatabaseService::recordToJson(const QSqlRecord &record)
{
    QJsonObject json;
    
    for (int i = 0; i < record.count(); ++i) {
        QString fieldName = record.fieldName(i);
        QVariant value = record.value(i);
        
        if (value.metaType() == QMetaType::fromType<QDateTime>()) {
            json[fieldName] = value.toDateTime().toString(Qt::ISODate);
        } else {
            json[fieldName] = QJsonValue::fromVariant(value);
        }
    }
    
    return json;
}

bool DatabaseService::resetDatabase()
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        emit operationCompleted("resetDatabase", false, "Database not connected");
        return false;
    }
    
    qDebug() << "Resetting database via DatabaseService";
    
    bool success = m_dbManager->resetDatabase();
    
    if (success) {
        // Refresh the material model if available
        if (m_materialModel) {
            m_materialModel->loadFromDatabase();
        }
        
        emit operationCompleted("resetDatabase", true, "Database reset successfully");
        emit dataChanged();
    } else {
        emit operationCompleted("resetDatabase", false, "Failed to reset database: " + m_dbManager->lastError());
    }
    
    return success;
}

bool DatabaseService::refreshDefaultMaterials()
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        emit operationCompleted("refreshDefaultMaterials", false, "Database not connected");
        return false;
    }
    
    qDebug() << "Refreshing default materials";
    
    bool success = m_dbManager->ensureDefaultData();
    
    if (success) {
        // Refresh the material model if available
        if (m_materialModel) {
            m_materialModel->loadFromDatabase();
        }
        
        emit operationCompleted("refreshDefaultMaterials", true, "Default materials refreshed successfully");
        emit dataChanged();
    } else {
        emit operationCompleted("refreshDefaultMaterials", false, "Failed to refresh default materials: " + m_dbManager->lastError());
    }
    
    return success;
}
