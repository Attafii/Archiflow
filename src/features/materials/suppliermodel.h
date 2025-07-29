#ifndef SUPPLIERMODEL_H
#define SUPPLIERMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QString>
#include <QList>

/**
 * @brief Represents a supplier entity
 */
struct Supplier {
    int id;
    QString name;
    QString contactPerson;
    QString email;
    QString phone;
    QString address;
    QString city;
    QString country;
    QString website;
    QString notes;
    bool isActive;

    Supplier()
        : id(0), isActive(true)
    {}

    Supplier(int id, const QString& name, const QString& contactPerson = "",
             const QString& email = "", const QString& phone = "",
             const QString& address = "", const QString& city = "",
             const QString& country = "", const QString& website = "",
             const QString& notes = "", bool isActive = true)
        : id(id), name(name), contactPerson(contactPerson), email(email),
          phone(phone), address(address), city(city), country(country),
          website(website), notes(notes), isActive(isActive)
    {}
};

/**
 * @brief Model class for managing supplier data
 */
class SupplierModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ID = 0,
        Name,
        ContactPerson,
        Email,
        Phone,
        Address,
        City,
        Country,
        Website,
        Notes,
        IsActive,
        ColumnCount
    };

    explicit SupplierModel(QObject *parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Supplier management methods
    bool addSupplier(const Supplier &supplier);
    bool updateSupplier(int row, const Supplier &supplier);
    bool removeSupplier(int row);
    Supplier supplier(int row) const;
    QList<Supplier> suppliers() const;
    
    // Utility methods
    void loadSampleData();
    void clear();
    int findSupplierById(int id) const;

signals:
    void supplierAdded(int row);
    void supplierUpdated(int row);
    void supplierRemoved(int row);

private:
    QList<Supplier> m_suppliers;
    int m_nextId;

    QString formatAddress(const Supplier &supplier) const;
};

#endif // SUPPLIERMODEL_H
