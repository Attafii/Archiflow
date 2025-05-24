#ifndef MATERIALMODEL_H
#define MATERIALMODEL_H

#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QPair>

class MaterialModel : public QSqlTableModel
{
    Q_OBJECT

public:
    enum MaterialRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        DescriptionRole,
        CategoryRole,
        QuantityRole,
        UnitRole,
        PriceRole
    };

    explicit MaterialModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());

    // Initialize the database and create tables if needed
    bool initialize();

    // Filter materials by search term and category
    void filterMaterials(const QString &searchTerm, const QString &category);

    // Clear all filters
    void clearFilters();

    // Get list of all categories
    QStringList getCategories();

    // CRUD operations
    bool addMaterial(const QString &name, const QString &description, 
                    const QString &category, int quantity, 
                    const QString &unit, double price);

    bool updateMaterial(int id, int newId, const QString &name, const QString &description, 
                       const QString &category, int quantity, 
                       const QString &unit, double price);

    bool deleteMaterial(int id);
    
    // CSV Import/Export
    bool importFromCSV(const QString &filePath, QString &errorMessage);
    bool exportToPDF(const QString &filePath, QString &errorMessage);
    
    // Get data for charts
    QList<QPair<QString, int>> getMaterialQuantitiesForChart();
    QList<QPair<QString, double>> getMaterialValuesForChart();

private:
    bool createTables();
};

#endif // MATERIALMODEL_H