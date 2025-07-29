#include "suppliermodel.h"
#include <QDebug>
#include <QColor>
#include <QFont>

SupplierModel::SupplierModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_nextId(1)
{
    loadSampleData();
}

int SupplierModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_suppliers.size();
}

int SupplierModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant SupplierModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_suppliers.size()) {
        return QVariant();
    }

    const Supplier &supplier = m_suppliers[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case ID: return supplier.id;
        case Name: return supplier.name;
        case ContactPerson: return supplier.contactPerson;
        case Email: return supplier.email;
        case Phone: return supplier.phone;
        case Address: return formatAddress(supplier);
        case City: return supplier.city;
        case Country: return supplier.country;
        case Website: return supplier.website;
        case Notes: return supplier.notes;
        case IsActive: return supplier.isActive ? "Yes" : "No";
        }
        break;

    case Qt::EditRole:
        switch (index.column()) {
        case ID: return supplier.id;
        case Name: return supplier.name;
        case ContactPerson: return supplier.contactPerson;
        case Email: return supplier.email;
        case Phone: return supplier.phone;
        case Address: return supplier.address;
        case City: return supplier.city;
        case Country: return supplier.country;
        case Website: return supplier.website;
        case Notes: return supplier.notes;
        case IsActive: return supplier.isActive;
        }
        break;

    case Qt::BackgroundRole:
        if (!supplier.isActive) {
            return QColor(255, 200, 200); // Light red for inactive suppliers
        }
        break;

    case Qt::FontRole:
        if (!supplier.isActive) {
            QFont font;
            font.setItalic(true);
            return font;
        }
        break;
    }

    return QVariant();
}

QVariant SupplierModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    case ID: return "ID";
    case Name: return "Name";
    case ContactPerson: return "Contact Person";
    case Email: return "Email";
    case Phone: return "Phone";
    case Address: return "Address";
    case City: return "City";
    case Country: return "Country";
    case Website: return "Website";
    case Notes: return "Notes";
    case IsActive: return "Active";
    }

    return QVariant();
}

bool SupplierModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_suppliers.size() || role != Qt::EditRole) {
        return false;
    }

    Supplier &supplier = m_suppliers[index.row()];

    switch (index.column()) {
    case Name: supplier.name = value.toString(); break;
    case ContactPerson: supplier.contactPerson = value.toString(); break;
    case Email: supplier.email = value.toString(); break;
    case Phone: supplier.phone = value.toString(); break;
    case Address: supplier.address = value.toString(); break;
    case City: supplier.city = value.toString(); break;
    case Country: supplier.country = value.toString(); break;
    case Website: supplier.website = value.toString(); break;
    case Notes: supplier.notes = value.toString(); break;
    case IsActive: supplier.isActive = value.toBool(); break;
    default: return false;
    }

    emit dataChanged(index, index, {role});
    emit supplierUpdated(index.row());
    return true;
}

Qt::ItemFlags SupplierModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    // ID column is not editable
    if (index.column() == ID) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool SupplierModel::addSupplier(const Supplier &supplier)
{
    beginInsertRows(QModelIndex(), m_suppliers.size(), m_suppliers.size());
    
    Supplier newSupplier = supplier;
    newSupplier.id = m_nextId++;
    m_suppliers.append(newSupplier);
    
    endInsertRows();
    
    emit supplierAdded(m_suppliers.size() - 1);
    qDebug() << "Added supplier:" << newSupplier.name;
    
    return true;
}

bool SupplierModel::updateSupplier(int row, const Supplier &supplier)
{
    if (row < 0 || row >= m_suppliers.size()) {
        return false;
    }

    // Preserve the original ID
    Supplier updatedSupplier = supplier;
    updatedSupplier.id = m_suppliers[row].id;
    
    m_suppliers[row] = updatedSupplier;
    
    QModelIndex topLeft = index(row, 0);
    QModelIndex bottomRight = index(row, ColumnCount - 1);
    emit dataChanged(topLeft, bottomRight);
    emit supplierUpdated(row);
    
    qDebug() << "Updated supplier:" << updatedSupplier.name;
    return true;
}

bool SupplierModel::removeSupplier(int row)
{
    if (row < 0 || row >= m_suppliers.size()) {
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row);
    QString supplierName = m_suppliers[row].name;
    m_suppliers.removeAt(row);
    endRemoveRows();
    
    emit supplierRemoved(row);
    qDebug() << "Removed supplier:" << supplierName;
    
    return true;
}

Supplier SupplierModel::supplier(int row) const
{
    if (row >= 0 && row < m_suppliers.size()) {
        return m_suppliers[row];
    }
    return Supplier();
}

QList<Supplier> SupplierModel::suppliers() const
{
    return m_suppliers;
}

void SupplierModel::loadSampleData()
{
    beginResetModel();
    m_suppliers.clear();
    m_nextId = 1;

    // Add sample suppliers
    m_suppliers << Supplier(m_nextId++, "BuildCorp Materials", "John Smith", 
                           "john.smith@buildcorp.com", "+1-555-0101",
                           "123 Industrial Blvd", "New York", "USA",
                           "www.buildcorp.com", "Primary concrete supplier", true);

    m_suppliers << Supplier(m_nextId++, "Steel Solutions Inc", "Maria Garcia",
                           "m.garcia@steelsolutions.com", "+1-555-0202",
                           "456 Steel Ave", "Chicago", "USA",
                           "www.steelsolutions.com", "Steel and metal products", true);

    m_suppliers << Supplier(m_nextId++, "Green Building Supply", "David Wilson",
                           "david@greenbuild.com", "+1-555-0303",
                           "789 Eco Way", "San Francisco", "USA",
                           "www.greenbuild.com", "Sustainable materials", true);

    m_suppliers << Supplier(m_nextId++, "Quality Lumber Co", "Sarah Johnson",
                           "sarah@qualitylumber.com", "+1-555-0404",
                           "321 Pine Street", "Seattle", "USA",
                           "www.qualitylumber.com", "Wood and lumber products", true);

    m_suppliers << Supplier(m_nextId++, "ProTech Hardware", "Michael Brown",
                           "mbrown@protech.com", "+1-555-0505",
                           "654 Tech Drive", "Austin", "USA",
                           "www.protech.com", "Hardware and fasteners", false);

    endResetModel();
    qDebug() << "Loaded" << m_suppliers.size() << "sample suppliers";
}

void SupplierModel::clear()
{
    beginResetModel();
    m_suppliers.clear();
    m_nextId = 1;
    endResetModel();
}

int SupplierModel::findSupplierById(int id) const
{
    for (int i = 0; i < m_suppliers.size(); ++i) {
        if (m_suppliers[i].id == id) {
            return i;
        }
    }
    return -1;
}

QString SupplierModel::formatAddress(const Supplier &supplier) const
{
    QStringList parts;
    if (!supplier.address.isEmpty()) parts << supplier.address;
    if (!supplier.city.isEmpty()) parts << supplier.city;
    if (!supplier.country.isEmpty()) parts << supplier.country;
    return parts.join(", ");
}
