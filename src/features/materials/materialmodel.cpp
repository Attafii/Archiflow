#include "materialmodel.h"
#include "../../database/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QColor>
#include <QIcon>
#include <QDateTime>

MaterialModel::MaterialModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_databaseManager(nullptr)
{
    loadFromDatabase();
}

int MaterialModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_filteredMaterials.size();
}

int MaterialModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant MaterialModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_filteredMaterials.size()) {
        return QVariant();
    }
    
    const Material &material = m_filteredMaterials.at(index.row());
    
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (index.column()) {
        case IdColumn:
            return material.id;
        case NameColumn:
            return material.name;
        case DescriptionColumn:
            return material.description;
        case CategoryColumn:
            return material.category;
        case QuantityColumn:
            return material.quantity;
        case UnitColumn:
            return material.unit;
        case PriceColumn:
            return QString("$%1").arg(material.price, 0, 'f', 2);
        case LocationColumn:
            return material.location;
        case StatusColumn:
            return material.status;
        default:
            return QVariant();
        }
        break;
        
    case Qt::BackgroundRole:
        // Highlight low stock items
        if (material.quantity <= material.reorderPoint) {
            return QColor("#FFE6E6"); // Light red background
        }
        // Alternate row colors
        return (index.row() % 2 == 0) ? QColor("#FFFFFF") : QColor("#F8F9FA");
        
    case Qt::ForegroundRole:
        if (material.status == "inactive") {
            return QColor("#6C757D"); // Gray text for inactive items
        } else if (material.quantity <= material.reorderPoint) {
            return QColor("#DC3545"); // Red text for low stock
        }
        return QColor("#495057"); // Default text color
        
    case Qt::DecorationRole:
        if (index.column() == StatusColumn) {
            if (material.status == "active") {
                return QIcon(":/icons/status-active.png");
            } else if (material.status == "inactive") {
                return QIcon(":/icons/status-inactive.png");
            } else if (material.status == "discontinued") {
                return QIcon(":/icons/status-discontinued.png");
            }
        }
        break;
        
    case Qt::ToolTipRole:
        if (index.column() == QuantityColumn && material.quantity <= material.reorderPoint) {
            return QString("Low stock! Current: %1, Reorder point: %2")
                   .arg(material.quantity).arg(material.reorderPoint);
        }
        break;
        
    case Qt::UserRole:
        // Return the full material object
        return QVariant::fromValue(material);
        
    default:
        break;
    }
    
    return QVariant();
}

QVariant MaterialModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case IdColumn:
            return "ID";
        case NameColumn:
            return "Name";
        case DescriptionColumn:
            return "Description";
        case CategoryColumn:
            return "Category";
        case QuantityColumn:
            return "Quantity";
        case UnitColumn:
            return "Unit";
        case PriceColumn:
            return "Price";
        case LocationColumn:
            return "Location";
        case StatusColumn:
            return "Status";
        default:
            return QVariant();
        }
    }
    
    return QAbstractTableModel::headerData(section, orientation, role);
}

bool MaterialModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_filteredMaterials.size() || role != Qt::EditRole) {
        return false;
    }
    
    Material &material = m_filteredMaterials[index.row()];
    
    switch (index.column()) {
    case NameColumn:
        material.name = value.toString();
        break;
    case DescriptionColumn:
        material.description = value.toString();
        break;
    case CategoryColumn:
        material.category = value.toString();
        break;
    case QuantityColumn:
        material.quantity = value.toInt();
        break;
    case UnitColumn:
        material.unit = value.toString();
        break;
    case PriceColumn:
        material.price = value.toDouble();
        break;
    case LocationColumn:
        material.location = value.toString();
        break;
    case StatusColumn:
        material.status = value.toString();
        break;
    default:
        return false;
    }
    
    material.updatedAt = QDateTime::currentDateTime();
    material.updatedBy = "current_user"; // TODO: Get actual current user
    
    emit QAbstractTableModel::dataChanged(index, index, {role});
    emit materialUpdated(material);
    
    return true;
}

Qt::ItemFlags MaterialModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    
    // Make most columns editable except ID
    if (index.column() != IdColumn) {
        flags |= Qt::ItemIsEditable;
    }
    
    return flags;
}

bool MaterialModel::addMaterial(const Material &material)
{
    Material newMaterial = material;
    
    // Set timestamps and user info
    newMaterial.createdAt = QDateTime::currentDateTime();
    newMaterial.updatedAt = QDateTime::currentDateTime();
    newMaterial.createdBy = "current_user"; // TODO: Get actual current user
    newMaterial.updatedBy = "current_user";
    
    // Add to database first
    if (m_databaseManager && m_databaseManager->isConnected()) {
        if (!addMaterialToDatabase(newMaterial)) {
            qDebug() << "Failed to add material to database";
            return false;
        }
        
        // Reload from database to get the actual ID assigned by the database
        if (!loadMaterialsFromDatabase()) {
            qDebug() << "Failed to reload materials after adding";
            return false;
        }
        
        // Apply current filters and refresh model
        filterMaterials();
        
        // Find the newly added material and emit signal
        for (const Material &mat : m_materials) {
            if (mat.name == newMaterial.name && 
                mat.category == newMaterial.category && 
                mat.createdAt >= newMaterial.createdAt.addSecs(-5)) {
                emit materialAdded(mat);
                break;
            }
        }
    } else {
        // Fallback to in-memory only
        beginInsertRows(QModelIndex(), m_materials.size(), m_materials.size());
        
        // Assign ID if not set
        if (newMaterial.id == 0) {
            newMaterial.id = getNextId();
        }
        
        m_materials.append(newMaterial);
        filterMaterials();
        
        endInsertRows();
        
        emit materialAdded(newMaterial);
    }
    
    emit dataRefreshed();
    return true;
}

void MaterialModel::removeMaterial(int row)
{
    if (row < 0 || row >= m_filteredMaterials.size()) {
        return;
    }
    
    const Material &material = m_filteredMaterials.at(row);
    
    // Delete from database first
    if (m_databaseManager && m_databaseManager->isConnected()) {
        if (!deleteMaterialFromDatabase(material.id)) {
            qDebug() << "Failed to delete material from database";
            return;
        }
        
        // Reload from database
        if (!loadMaterialsFromDatabase()) {
            qDebug() << "Failed to reload materials after deleting";
            return;
        }
        
        // Apply current filters and refresh model
        filterMaterials();
        
        emit materialRemoved(material.id);
    } else {
        // Fallback to in-memory only
        beginRemoveRows(QModelIndex(), row, row);
        
        int originalIndex = m_materials.indexOf(material);
        if (originalIndex >= 0) {
            m_materials.removeAt(originalIndex);
            emit materialRemoved(material.id);
        }
        
        filterMaterials();
        endRemoveRows();
    }
    
    emit dataRefreshed();
}

bool MaterialModel::updateMaterial(int row, const Material &material)
{
    if (row < 0 || row >= m_filteredMaterials.size()) {
        return false;
    }
    
    const Material &oldMaterial = m_filteredMaterials.at(row);
    Material updatedMaterial = material;
    updatedMaterial.id = oldMaterial.id; // Preserve the original ID
    updatedMaterial.createdAt = oldMaterial.createdAt; // Preserve creation time
    updatedMaterial.updatedAt = QDateTime::currentDateTime();
    updatedMaterial.updatedBy = "current_user";
    
    // Update in database first
    if (m_databaseManager && m_databaseManager->isConnected()) {
        if (!updateMaterialInDatabase(updatedMaterial)) {
            qDebug() << "Failed to update material in database";
            return false;
        }
        
        // Reload from database
        if (!loadMaterialsFromDatabase()) {
            qDebug() << "Failed to reload materials after updating";
            return false;
        }
        
        // Apply current filters and refresh model
        filterMaterials();
        
        emit materialUpdated(updatedMaterial);
    } else {
        // Fallback to in-memory only
        int originalIndex = m_materials.indexOf(oldMaterial);
        if (originalIndex >= 0) {
            m_materials[originalIndex] = updatedMaterial;
            filterMaterials();
            
            emit QAbstractTableModel::dataChanged(index(row, 0), index(row, ColumnCount - 1));
            emit materialUpdated(updatedMaterial);
        } else {
            return false;
        }
    }
    
    emit dataRefreshed();
    return true;
}

Material MaterialModel::getMaterial(int row) const
{
    if (row >= 0 && row < m_filteredMaterials.size()) {
        return m_filteredMaterials.at(row);
    }
    return Material();
}

bool MaterialModel::loadFromDatabase()
{
    beginResetModel();
    m_materials.clear();
    
    // Try to load from actual database first
    if (m_databaseManager && m_databaseManager->isConnected()) {
        if (loadMaterialsFromDatabase()) {
            qDebug() << "Loaded" << m_materials.size() << "materials from database";
        } else {
            qDebug() << "Database connected but no materials found";
        }
    } else {
        qDebug() << "Database not available, loading sample data as fallback";
        loadSampleMaterialsData();
    }
    
    // Apply current filters
    filterMaterials();
    endResetModel();
    
    emit dataRefreshed();
    return true;
}

void MaterialModel::setDatabaseManager(DatabaseManager *dbManager)
{
    m_databaseManager = dbManager;
    // Reload data from the database when the manager is set
    if (m_databaseManager && m_databaseManager->isConnected()) {
        loadFromDatabase();
    }
}

bool MaterialModel::loadMaterialsFromDatabase()
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return false;
    }
    
    QSqlQuery query = m_databaseManager->executeQuery(
        "SELECT id, name, description, category, quantity, unit, price, supplier_id, "
        "barcode, location, minimum_stock, maximum_stock, reorder_point, status, "
        "created_at, updated_at, created_by, updated_by FROM materials ORDER BY name"
    );
    
    while (query.next()) {
        Material material;
        material.id = query.value("id").toInt();
        material.name = query.value("name").toString();
        material.description = query.value("description").toString();
        material.category = query.value("category").toString();
        material.quantity = query.value("quantity").toInt();
        material.unit = query.value("unit").toString();
        material.price = query.value("price").toDouble();
        material.supplierId = query.value("supplier_id").toInt();
        material.barcode = query.value("barcode").toString();
        material.location = query.value("location").toString();
        material.minimumStock = query.value("minimum_stock").toInt();
        material.maximumStock = query.value("maximum_stock").toInt();
        material.reorderPoint = query.value("reorder_point").toInt();
        material.status = query.value("status").toString();
        material.createdAt = query.value("created_at").toDateTime();
        material.updatedAt = query.value("updated_at").toDateTime();
        material.createdBy = query.value("created_by").toString();
        material.updatedBy = query.value("updated_by").toString();
        
        m_materials.append(material);
    }
    
    return true;
}

bool MaterialModel::addMaterialToDatabase(const Material &material)
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return false;
    }
    
    QString query = R"(
        INSERT INTO materials (name, description, category, quantity, unit, price, 
                             supplier_id, barcode, location, minimum_stock, maximum_stock, 
                             reorder_point, status, created_by, updated_by)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    QVariantList params = {
        material.name, material.description, material.category, material.quantity,
        material.unit, material.price, material.supplierId, material.barcode,
        material.location, material.minimumStock, material.maximumStock,
        material.reorderPoint, material.status, material.createdBy, material.updatedBy
    };
    
    return m_databaseManager->executeNonQuery(query, params);
}

bool MaterialModel::updateMaterialInDatabase(const Material &material)
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return false;
    }
    
    QString query = R"(
        UPDATE materials SET 
            name = ?, description = ?, category = ?, quantity = ?, unit = ?, 
            price = ?, supplier_id = ?, barcode = ?, location = ?, 
            minimum_stock = ?, maximum_stock = ?, reorder_point = ?, 
            status = ?, updated_at = CURRENT_TIMESTAMP, updated_by = ?
        WHERE id = ?
    )";
    
    QVariantList params = {
        material.name, material.description, material.category, material.quantity,
        material.unit, material.price, material.supplierId, material.barcode,
        material.location, material.minimumStock, material.maximumStock,
        material.reorderPoint, material.status, material.updatedBy, material.id
    };
    
    return m_databaseManager->executeNonQuery(query, params);
}

bool MaterialModel::deleteMaterialFromDatabase(int materialId)
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return false;
    }
    
    QString query = "DELETE FROM materials WHERE id = ?";
    return m_databaseManager->executeNonQuery(query, {materialId});
}

void MaterialModel::loadSampleMaterialsData()
{
    Material concrete;
    concrete.id = 1;
    concrete.name = "Concrete";
    concrete.description = "Standard concrete mix for foundations";
    concrete.category = "Construction Materials";
    concrete.quantity = 100;
    concrete.unit = "m³";
    concrete.price = 150.0;
    concrete.location = "Warehouse A";
    concrete.minimumStock = 20;
    concrete.maximumStock = 200;
    concrete.reorderPoint = 30;
    concrete.status = "active";
    concrete.createdAt = QDateTime::currentDateTime().addDays(-30);
    concrete.updatedAt = QDateTime::currentDateTime().addDays(-5);
    concrete.createdBy = "Admin";
    concrete.updatedBy = "Manager";
    m_materials.append(concrete);
    
    Material steel;
    steel.id = 2;
    steel.name = "Steel Rebar";
    steel.description = "Reinforcement steel bars 16mm";
    steel.category = "Metal";
    steel.quantity = 50;
    steel.unit = "tons";
    steel.price = 800.0;
    steel.location = "Warehouse B";
    steel.minimumStock = 10;
    steel.maximumStock = 100;
    steel.reorderPoint = 15;
    steel.status = "active";
    steel.createdAt = QDateTime::currentDateTime().addDays(-25);
    steel.updatedAt = QDateTime::currentDateTime().addDays(-3);
    steel.createdBy = "Admin";
    steel.updatedBy = "Supervisor";
    m_materials.append(steel);
    
    Material cement;
    cement.id = 3;
    cement.name = "Portland Cement";
    cement.description = "High-grade portland cement bags";
    cement.category = "Construction Materials";
    cement.quantity = 5; // Low stock
    cement.unit = "bags";
    cement.price = 25.0;
    cement.location = "Storage Room 1";
    cement.minimumStock = 20;
    cement.maximumStock = 500;
    cement.reorderPoint = 25;
    cement.status = "active";
    cement.createdAt = QDateTime::currentDateTime().addDays(-20);
    cement.updatedAt = QDateTime::currentDateTime().addDays(-1);
    cement.createdBy = "Admin";
    cement.updatedBy = "Manager";
    m_materials.append(cement);
    
    Material bricks;
    bricks.id = 4;
    bricks.name = "Red Bricks";
    bricks.description = "Standard clay bricks for construction";
    bricks.category = "Masonry";
    bricks.quantity = 10000;
    bricks.unit = "pcs";
    bricks.price = 0.5;
    bricks.location = "Yard A";
    bricks.minimumStock = 5000;
    bricks.maximumStock = 50000;
    bricks.reorderPoint = 7500;
    bricks.status = "active";
    bricks.createdAt = QDateTime::currentDateTime().addDays(-15);
    bricks.updatedAt = QDateTime::currentDateTime().addDays(-2);
    bricks.createdBy = "Supervisor";
    bricks.updatedBy = "Admin";
    m_materials.append(bricks);
    
    Material paint;
    paint.id = 5;
    paint.name = "Exterior Paint";
    paint.description = "Weather-resistant exterior paint";
    paint.category = "Finishes";
    paint.quantity = 2; // Low stock
    paint.unit = "gallons";
    paint.price = 45.0;
    paint.location = "Storage Room 2";
    paint.minimumStock = 10;
    paint.maximumStock = 100;
    paint.reorderPoint = 15;
    paint.status = "active";
    paint.createdAt = QDateTime::currentDateTime().addDays(-10);
    paint.updatedAt = QDateTime::currentDateTime();
    paint.createdBy = "Manager";
    paint.updatedBy = "Manager";
    m_materials.append(paint);
    
    Material tiles;
    tiles.id = 6;
    tiles.name = "Ceramic Tiles";
    tiles.description = "Premium ceramic floor tiles 60x60cm";
    tiles.category = "Finishes";
    tiles.quantity = 500;
    tiles.unit = "m²";
    tiles.price = 35.0;
    tiles.location = "Warehouse C";
    tiles.minimumStock = 100;
    tiles.maximumStock = 1000;
    tiles.reorderPoint = 150;
    tiles.status = "active";
    tiles.createdAt = QDateTime::currentDateTime().addDays(-8);
    tiles.updatedAt = QDateTime::currentDateTime().addDays(-1);
    tiles.createdBy = "Admin";
    tiles.updatedBy = "Supervisor";
    m_materials.append(tiles);
    
    Material insulation;
    insulation.id = 7;
    insulation.name = "Thermal Insulation";
    insulation.description = "Rockwool thermal insulation panels";
    insulation.category = "Insulation";
    insulation.quantity = 200;
    insulation.unit = "m²";
    insulation.price = 15.0;
    insulation.location = "Warehouse A";
    insulation.minimumStock = 50;
    insulation.maximumStock = 500;
    insulation.reorderPoint = 75;
    insulation.status = "active";
    insulation.createdAt = QDateTime::currentDateTime().addDays(-12);
    insulation.updatedAt = QDateTime::currentDateTime().addDays(-4);
    insulation.createdBy = "Supervisor";    insulation.updatedBy = "Manager";
    m_materials.append(insulation);
    
    Material lumber;
    lumber.id = 8;
    lumber.name = "Pine Lumber";
    lumber.description = "Construction grade pine lumber 2x4x8ft";
    lumber.category = "Wood";
    lumber.quantity = 300;
    lumber.unit = "pieces";
    lumber.price = 12.50;
    lumber.location = "Lumber Yard";
    lumber.minimumStock = 100;
    lumber.maximumStock = 1000;
    lumber.reorderPoint = 150;
    lumber.status = "active";
    lumber.createdAt = QDateTime::currentDateTime().addDays(-18);
    lumber.updatedAt = QDateTime::currentDateTime().addDays(-6);
    lumber.createdBy = "Admin";
    lumber.updatedBy = "Supervisor";
    m_materials.append(lumber);
    
    Material pipe;
    pipe.id = 9;
    pipe.name = "PVC Pipe";
    pipe.description = "PVC pipe 4 inch diameter for drainage";
    pipe.category = "Plumbing";
    pipe.quantity = 8; // Low stock
    pipe.unit = "meters";
    pipe.price = 18.0;
    pipe.location = "Storage Room 3";
    pipe.minimumStock = 20;
    pipe.maximumStock = 200;
    pipe.reorderPoint = 25;
    pipe.status = "active";
    pipe.createdAt = QDateTime::currentDateTime().addDays(-22);
    pipe.updatedAt = QDateTime::currentDateTime().addDays(-7);
    pipe.createdBy = "Manager";
    pipe.updatedBy = "Admin";
    m_materials.append(pipe);
    
    Material electrical;
    electrical.id = 10;
    electrical.name = "Electrical Wire";
    electrical.description = "12 AWG copper electrical wire";
    electrical.category = "Electrical";
    electrical.quantity = 500;
    electrical.unit = "feet";
    electrical.price = 2.25;
    electrical.location = "Electrical Storage";
    electrical.minimumStock = 200;
    electrical.maximumStock = 2000;
    electrical.reorderPoint = 300;
    electrical.status = "active";
    electrical.createdAt = QDateTime::currentDateTime().addDays(-14);
    electrical.updatedAt = QDateTime::currentDateTime().addDays(-2);
    electrical.createdBy = "Supervisor";
    electrical.updatedBy = "Manager";
    m_materials.append(electrical);
    
    endResetModel();
    
    // Apply filtering after model reset    filterMaterials();
    
    qDebug() << "Loaded" << m_materials.size() << "sample materials";
}

bool MaterialModel::saveToDatabase()
{
    // TODO: Implement database saving
    // This would involve INSERT/UPDATE/DELETE operations
    // based on the changes tracked in the model
    
    qDebug() << "Material model save to database not yet implemented";
    return true;
}

void MaterialModel::clearAllMaterials()
{
    // Clear from database first
    if (m_databaseManager && m_databaseManager->isConnected()) {
        if (!m_databaseManager->executeNonQuery("DELETE FROM materials")) {
            qDebug() << "Failed to clear materials from database";
            return;
        }
        
        // Also clear material movements
        m_databaseManager->executeNonQuery("DELETE FROM material_movements");
        
        qDebug() << "All materials cleared from database";
    }
    
    // Clear from model
    beginResetModel();
    m_materials.clear();
    m_filteredMaterials.clear();
    endResetModel();
    
    emit dataRefreshed();
    qDebug() << "All materials cleared from model";
}

void MaterialModel::loadSampleData()
{
    beginResetModel();
    m_materials.clear();
    
    // Add comprehensive sample materials for testing
    Material concrete;
    concrete.id = 1;
    concrete.name = "Concrete";
    concrete.description = "Standard concrete mix for foundations";
    concrete.category = "Construction";
    concrete.quantity = 100;
    concrete.unit = "m³";
    concrete.price = 150.0;
    concrete.location = "Warehouse A";
    concrete.minimumStock = 20;
    concrete.maximumStock = 200;
    concrete.reorderPoint = 30;
    concrete.status = "active";
    concrete.createdAt = QDateTime::currentDateTime().addDays(-30);
    concrete.updatedAt = QDateTime::currentDateTime().addDays(-5);
    concrete.createdBy = "System";
    concrete.updatedBy = "System";
    m_materials.append(concrete);
    
    Material steel;
    steel.id = 2;
    steel.name = "Steel Rebar";
    steel.description = "Reinforcement steel bars 16mm";
    steel.category = "Metal";
    steel.quantity = 50;
    steel.unit = "tons";
    steel.price = 800.0;
    steel.location = "Warehouse B";
    steel.minimumStock = 10;
    steel.maximumStock = 100;
    steel.reorderPoint = 15;
    steel.status = "active";
    steel.createdAt = QDateTime::currentDateTime().addDays(-25);
    steel.updatedAt = QDateTime::currentDateTime().addDays(-3);
    steel.createdBy = "System";
    steel.updatedBy = "System";
    m_materials.append(steel);
    
    Material pipes;
    pipes.id = 3;
    pipes.name = "PVC Pipes";
    pipes.description = "Standard PVC pipes 110mm diameter";
    pipes.category = "Plumbing";
    pipes.quantity = 200;
    pipes.unit = "meters";
    pipes.price = 12.0;
    pipes.location = "Storage Area 1";
    pipes.minimumStock = 50;
    pipes.maximumStock = 500;
    pipes.reorderPoint = 75;
    pipes.status = "active";
    pipes.createdAt = QDateTime::currentDateTime().addDays(-20);
    pipes.updatedAt = QDateTime::currentDateTime().addDays(-7);
    pipes.createdBy = "System";
    pipes.updatedBy = "System";
    m_materials.append(pipes);
    
    Material bricks;
    bricks.id = 4;
    bricks.name = "Red Bricks";
    bricks.description = "Standard clay bricks for construction";
    bricks.category = "Construction";
    bricks.quantity = 10000;
    bricks.unit = "pieces";
    bricks.price = 0.5;
    bricks.location = "Yard A";
    bricks.minimumStock = 5000;
    bricks.maximumStock = 50000;
    bricks.reorderPoint = 7500;
    bricks.status = "active";
    bricks.createdAt = QDateTime::currentDateTime().addDays(-15);
    bricks.updatedAt = QDateTime::currentDateTime().addDays(-2);
    bricks.createdBy = "System";
    bricks.updatedBy = "System";
    m_materials.append(bricks);
    
    Material paint;
    paint.id = 5;
    paint.name = "Exterior Paint";
    paint.description = "Weather-resistant exterior paint";
    paint.category = "Other";
    paint.quantity = 25;
    paint.unit = "liters";
    paint.price = 45.0;
    paint.location = "Storage Room 2";
    paint.minimumStock = 10;
    paint.maximumStock = 100;
    paint.reorderPoint = 15;
    paint.status = "active";
    paint.createdAt = QDateTime::currentDateTime().addDays(-10);
    paint.updatedAt = QDateTime::currentDateTime();
    paint.createdBy = "System";
    paint.updatedBy = "System";
    m_materials.append(paint);
    
    filterMaterials();
    endResetModel();
    
    emit dataRefreshed();
    qDebug() << "Loaded" << m_materials.size() << "sample materials";
}

void MaterialModel::setFilter(const QString &filter)
{
    m_nameFilter = filter;
    filterMaterials();
}

void MaterialModel::setCategoryFilter(const QString &category)
{
    m_categoryFilter = category;
    filterMaterials();
}

void MaterialModel::setStatusFilter(const QString &status)
{
    m_statusFilter = status;
    filterMaterials();
}

int MaterialModel::getTotalMaterials() const
{
    return m_materials.size();
}

int MaterialModel::getLowStockCount() const
{
    int count = 0;
    for (const Material &material : m_materials) {
        if (material.quantity <= material.reorderPoint) {
            count++;
        }
    }
    return count;
}

double MaterialModel::getTotalValue() const
{
    double total = 0.0;
    for (const Material &material : m_materials) {
        total += material.quantity * material.price;
    }
    return total;
}

QStringList MaterialModel::getCategories() const
{
    QSet<QString> categories;
    for (const Material &material : m_materials) {
        if (!material.category.isEmpty()) {
            categories.insert(material.category);
        }
    }
    
    QStringList sortedCategories = QStringList(categories.begin(), categories.end());
    sortedCategories.sort();
    return sortedCategories;
}

void MaterialModel::refresh()
{
    loadFromDatabase();
}

void MaterialModel::filterMaterials()
{
    beginResetModel();
    
    m_filteredMaterials.clear();
    
    for (const Material &material : m_materials) {
        if (matchesFilter(material)) {
            m_filteredMaterials.append(material);
        }
    }
    
    endResetModel();
}

bool MaterialModel::matchesFilter(const Material &material) const
{
    // Name filter
    if (!m_nameFilter.isEmpty()) {
        bool nameMatch = material.name.contains(m_nameFilter, Qt::CaseInsensitive) ||
                        material.description.contains(m_nameFilter, Qt::CaseInsensitive);
        if (!nameMatch) {
            return false;
        }
    }
    
    // Category filter
    if (!m_categoryFilter.isEmpty() && m_categoryFilter != "All") {
        if (material.category != m_categoryFilter) {
            return false;
        }
    }
    
    // Status filter
    if (!m_statusFilter.isEmpty() && m_statusFilter != "All") {
        if (material.status != m_statusFilter) {
            return false;
        }
    }
    
    return true;
}

int MaterialModel::getNextId() const
{
    int maxId = 0;
    for (const Material &material : m_materials) {
        if (material.id > maxId) {
            maxId = material.id;
        }
    }
    return maxId + 1;
}
