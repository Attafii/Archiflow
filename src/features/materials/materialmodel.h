#ifndef MATERIALMODEL_H
#define MATERIALMODEL_H

#include <QAbstractTableModel>
#include <QDate>
#include <QVariant>

/**
 * @brief Material data structure
 */
struct Material
{
    int id;
    QString name;
    QString description;
    QString category;
    int quantity;
    QString unit;
    double price;
    int supplierId;
    QString barcode;
    QString location;
    int minimumStock;
    int maximumStock;
    int reorderPoint;
    QDateTime createdAt;
    QDateTime updatedAt;
    QString createdBy;
    QString updatedBy;
    QString status; // active, inactive, discontinued
      Material() 
        : id(0), quantity(0), price(0.0), supplierId(0)
        , minimumStock(0), maximumStock(1000), reorderPoint(10)
        , status("active") {}
    
    // Equality operator for QList operations
    bool operator==(const Material &other) const {
        return id == other.id;
    }
};

/**
 * @brief The MaterialModel class provides a table model for materials management
 */
class MaterialModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        IdColumn = 0,
        NameColumn,
        DescriptionColumn,
        CategoryColumn,
        QuantityColumn,
        UnitColumn,
        PriceColumn,
        LocationColumn,
        StatusColumn,
        ColumnCount
    };

    explicit MaterialModel(QObject *parent = nullptr);
    
    // QAbstractTableModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
      // Custom methods
    bool addMaterial(const Material &material);
    void removeMaterial(int row);
    bool updateMaterial(int row, const Material &material);
    Material getMaterial(int row) const;
    int getNextId() const;
      // Database operations
    bool loadFromDatabase();
    bool saveToDatabase();
    void clearAllMaterials();
    void loadSampleData();
    
    // Real-time database operations
    bool addMaterialToDatabase(const Material &material);
    bool updateMaterialInDatabase(const Material &material);
    bool deleteMaterialFromDatabase(int materialId);
    bool loadMaterialsFromDatabase();
    void loadSampleMaterialsData();
    
    // Database connection
    void setDatabaseManager(class DatabaseManager *dbManager);
    
    // Search and filter
    void setFilter(const QString &filter);
    void setCategoryFilter(const QString &category);
    void setStatusFilter(const QString &status);
    
    // Statistics
    int getTotalMaterials() const;
    int getLowStockCount() const;
    double getTotalValue() const;
    QStringList getCategories() const;

public slots:
    void refresh();

signals:
    void dataRefreshed();
    void materialAdded(const Material &material);
    void materialRemoved(int id);
    void materialUpdated(const Material &material);

private:
    void filterMaterials();
    bool matchesFilter(const Material &material) const;
      QList<Material> m_materials;
    QList<Material> m_filteredMaterials;
    QString m_nameFilter;
    QString m_categoryFilter;
    QString m_statusFilter;
    
    class DatabaseManager *m_databaseManager;
};

Q_DECLARE_METATYPE(Material)

#endif // MATERIALMODEL_H
