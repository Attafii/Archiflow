#include "materialmodel.h"
#include <QSqlRecord>
#include <QtPrintSupport/QPrinter>
#include <QPainter>
#include <QDateTime>
#include <QPageSize>
#include <QPageLayout>

MaterialModel::MaterialModel(QObject *parent, QSqlDatabase db)
    : QSqlTableModel(parent, db)
{
    setTable("materials");
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    
    // Set header data
    setHeaderData(0, Qt::Horizontal, tr("ID"));
    setHeaderData(1, Qt::Horizontal, tr("Name"));
    setHeaderData(2, Qt::Horizontal, tr("Description"));
    setHeaderData(3, Qt::Horizontal, tr("Category"));
    setHeaderData(4, Qt::Horizontal, tr("Quantity"));
    setHeaderData(5, Qt::Horizontal, tr("Unit"));
    setHeaderData(6, Qt::Horizontal, tr("Price"));
    
    select();
}

bool MaterialModel::initialize()
{
    // Check if database is open
    if (!database().isOpen()) {
        qDebug() << "Database is not open";
        return false;
    }
    
    // Create tables if they don't exist
    if (!createTables()) {
        qDebug() << "Failed to create tables";
        return false;
    }
    
    return true;
}

bool MaterialModel::createTables()
{
    QSqlQuery query;
    
    // Create materials table
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS materials ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "description TEXT, "
        "category TEXT, "
        "quantity INTEGER DEFAULT 0, "
        "unit TEXT, "
        "price REAL DEFAULT 0.0"
        ")");
    
    if (!success) {
        qDebug() << "Error creating materials table:" << query.lastError().text();
        return false;
    }
    
    // Create categories table for future use
    success = query.exec(
        "CREATE TABLE IF NOT EXISTS categories ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL UNIQUE"
        ")");
    
    if (!success) {
        qDebug() << "Error creating categories table:" << query.lastError().text();
        return false;
    }
    
    return true;
}

void MaterialModel::filterMaterials(const QString &searchTerm, const QString &category)
{
    QString filter;
    
    // Apply search term filter
    if (!searchTerm.isEmpty()) {
        filter = QString("name LIKE '%%1%' OR description LIKE '%%1%'").arg(searchTerm);
    }
    
    // Apply category filter
    if (!category.isEmpty() && category != "All Categories") {
        if (!filter.isEmpty()) {
            filter += " AND ";
        }
        filter += QString("category = '%1'").arg(category);
    }
    
    // Set the filter
    setFilter(filter);
    select();
}

void MaterialModel::clearFilters()
{
    setFilter("");
    select();
}

QStringList MaterialModel::getCategories()
{
    QStringList categories;
    QSqlQuery query("SELECT DISTINCT category FROM materials ORDER BY category");
    
    while (query.next()) {
        QString category = query.value(0).toString();
        if (!category.isEmpty()) {
            categories << category;
        }
    }
    
    return categories;
}

bool MaterialModel::addMaterial(const QString &name, const QString &description, 
                              const QString &category, int quantity, 
                              const QString &unit, double price)
{
    QSqlRecord record = this->record();
    record.remove(0); // Remove ID field as it's auto-incremented
    
    record.setValue("name", name);
    record.setValue("description", description);
    record.setValue("category", category);
    record.setValue("quantity", quantity);
    record.setValue("unit", unit);
    record.setValue("price", price);
    
    if (insertRecord(-1, record)) {
        return submitAll();
    }
    
    return false;
}

bool MaterialModel::updateMaterial(int id, int newId, const QString &name, const QString &description, 
                                 const QString &category, int quantity, 
                                 const QString &unit, double price)
{
    // Find the record with the given ID
    for (int i = 0; i < rowCount(); ++i) {
        if (data(index(i, 0)).toInt() == id) {
            // Update ID if it has changed
            if (id != newId) {
                setData(index(i, 0), newId);
            }
            setData(index(i, 1), name);
            setData(index(i, 2), description);
            setData(index(i, 3), category);
            setData(index(i, 4), quantity);
            setData(index(i, 5), unit);
            setData(index(i, 6), price);
            
            return submitAll();
        }
    }
    
    return false;
}

bool MaterialModel::deleteMaterial(int id)
{
    // Find the record with the given ID
    for (int i = 0; i < rowCount(); ++i) {
        if (data(index(i, 0)).toInt() == id) {
            removeRow(i);
            return submitAll();
        }
    }
    
    return false;
}

bool MaterialModel::importFromCSV(const QString &filePath, QString &errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorMessage = tr("Could not open file: %1").arg(file.errorString());
        return false;
    }
    
    QTextStream in(&file);
    
    // Read header line
    if (in.atEnd()) {
        errorMessage = tr("File is empty");
        file.close();
        return false;
    }
    
    // Skip header line
    in.readLine();
    
    // Begin transaction for better performance
    database().transaction();
    
    int lineNumber = 1;
    int successCount = 0;
    
    while (!in.atEnd()) {
        lineNumber++;
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        
        QStringList fields = line.split(',');
        
        // Check if we have the correct number of fields
        if (fields.size() < 6) {
            errorMessage = tr("Line %1: Not enough fields").arg(lineNumber);
            database().rollback();
            file.close();
            return false;
        }
        
        // Extract fields
        QString name = fields[0].trimmed();
        QString description = fields[1].trimmed();
        QString category = fields[2].trimmed();
        bool quantityOk;
        int quantity = fields[3].trimmed().toInt(&quantityOk);
        if (!quantityOk) {
            errorMessage = tr("Line %1: Invalid quantity").arg(lineNumber);
            database().rollback();
            file.close();
            return false;
        }
        
        QString unit = fields[4].trimmed();
        bool priceOk;
        double price = fields[5].trimmed().toDouble(&priceOk);
        if (!priceOk) {
            errorMessage = tr("Line %1: Invalid price").arg(lineNumber);
            database().rollback();
            file.close();
            return false;
        }
        
        // Add material to database
        if (addMaterial(name, description, category, quantity, unit, price)) {
            successCount++;
        }
    }
    
    file.close();
    
    if (successCount > 0) {
        if (database().commit()) {
            errorMessage = tr("%1 materials imported successfully").arg(successCount);
            return true;
        } else {
            database().rollback();
            errorMessage = tr("Database error: %1").arg(database().lastError().text());
            return false;
        }
    } else {
        database().rollback();
        errorMessage = tr("No materials were imported");
        return false;
    }
}

QList<QPair<QString, int>> MaterialModel::getMaterialQuantitiesForChart()
{
    QList<QPair<QString, int>> result;
    QMap<QString, int> categoryQuantities;
    
    // Calculate total quantity for each category
    for (int i = 0; i < rowCount(); ++i) {
        QString category = data(index(i, 3)).toString();
        int quantity = data(index(i, 4)).toInt();
        
        if (categoryQuantities.contains(category)) {
            categoryQuantities[category] += quantity;
        } else {
            categoryQuantities[category] = quantity;
        }
    }
    
    // Convert map to list of pairs
    QMapIterator<QString, int> it(categoryQuantities);
    while (it.hasNext()) {
        it.next();
        result.append(qMakePair(it.key(), it.value()));
    }
    
    return result;
}

QList<QPair<QString, double>> MaterialModel::getMaterialValuesForChart()
{
    QList<QPair<QString, double>> result;
    QMap<QString, double> categoryValues;
    
    // Calculate total value for each category
    for (int i = 0; i < rowCount(); ++i) {
        QString category = data(index(i, 3)).toString();
        int quantity = data(index(i, 4)).toInt();
        double price = data(index(i, 6)).toDouble();
        double value = quantity * price;
        
        if (categoryValues.contains(category)) {
            categoryValues[category] += value;
        } else {
            categoryValues[category] = value;
        }
    }
    
    // Convert map to list of pairs
    QMapIterator<QString, double> it(categoryValues);
    while (it.hasNext()) {
        it.next();
        result.append(qMakePair(it.key(), it.value()));
    }
    
    return result;
}

bool MaterialModel::exportToPDF(const QString &filePath, QString &errorMessage)
{
    // Create a printer
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);
    
    // Create a painter to paint on the printer
    QPainter painter;
    if (!painter.begin(&printer)) {
        errorMessage = tr("Could not open PDF file for writing.");
        return false;
    }
    
    // Clear any filters to export all materials
    QString oldFilter = filter();
    setFilter("");
    select();
    
    int rowCount = this->rowCount();
    
    // Set up fonts
    QFont titleFont("Arial", 18, QFont::Bold);
    QFont headerFont("Arial", 12, QFont::Bold);
    QFont contentFont("Arial", 10);
    
    // Calculate page metrics
    int pageWidth = printer.pageRect(QPrinter::Point).width();
    int pageHeight = printer.pageRect(QPrinter::Point).height();
    int yPos = 100;
    int margin = 40;
    
    // Draw title
    painter.setFont(titleFont);
    painter.drawText(margin, yPos, tr("Materials Report"));
    yPos += 50;
    
    // Draw date
    painter.setFont(contentFont);
    painter.drawText(margin, yPos, tr("Generated on: %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
    yPos += 40;
    
    // Draw table headers
    painter.setFont(headerFont);
    int colWidth = (pageWidth - 2 * margin) / 7;
    
    painter.drawText(margin, yPos, colWidth, 30, Qt::AlignLeft | Qt::AlignVCenter, tr("ID"));
    painter.drawText(margin + colWidth, yPos, colWidth, 30, Qt::AlignLeft | Qt::AlignVCenter, tr("Name"));
    painter.drawText(margin + 2 * colWidth, yPos, colWidth, 30, Qt::AlignLeft | Qt::AlignVCenter, tr("Category"));
    painter.drawText(margin + 3 * colWidth, yPos, colWidth, 30, Qt::AlignLeft | Qt::AlignVCenter, tr("Quantity"));
    painter.drawText(margin + 4 * colWidth, yPos, colWidth, 30, Qt::AlignLeft | Qt::AlignVCenter, tr("Unit"));
    painter.drawText(margin + 5 * colWidth, yPos, colWidth, 30, Qt::AlignLeft | Qt::AlignVCenter, tr("Price"));
    painter.drawText(margin + 6 * colWidth, yPos, colWidth, 30, Qt::AlignLeft | Qt::AlignVCenter, tr("Total Value"));
    
    yPos += 30;
    
    // Draw horizontal line
    painter.drawLine(margin, yPos, pageWidth - margin, yPos);
    yPos += 10;
    
    // Draw table content
    painter.setFont(contentFont);
    int rowHeight = 25;
    double totalValue = 0.0;
    
    for (int i = 0; i < rowCount; ++i) {
        // Check if we need a new page
        if (yPos > pageHeight - margin) {
            printer.newPage();
            yPos = margin + 50;
            
            // Redraw headers on new page
            painter.setFont(headerFont);
            painter.drawText(margin, yPos, colWidth, 30, Qt::AlignLeft | Qt::AlignVCenter, tr("ID"));
            painter.drawText(margin + colWidth, yPos, colWidth, 30, Qt::AlignLeft | Qt::AlignVCenter, tr("Name"));
            painter.drawText(margin + 2 * colWidth, yPos, colWidth, 30, Qt::AlignLeft | Qt::AlignVCenter, tr("Category"));
            painter.drawText(margin + 3 * colWidth, yPos, colWidth, 30, Qt::AlignLeft | Qt::AlignVCenter, tr("Quantity"));
            painter.drawText(margin + 4 * colWidth, yPos, colWidth, 30, Qt::AlignLeft | Qt::AlignVCenter, tr("Unit"));
            painter.drawText(margin + 5 * colWidth, yPos, colWidth, 30, Qt::AlignLeft | Qt::AlignVCenter, tr("Price"));
            painter.drawText(margin + 6 * colWidth, yPos, colWidth, 30, Qt::AlignLeft | Qt::AlignVCenter, tr("Total Value"));
            
            yPos += 30;
            painter.drawLine(margin, yPos, pageWidth - margin, yPos);
            yPos += 10;
            painter.setFont(contentFont);
        }
        
        int id = data(index(i, 0)).toInt();
        QString name = data(index(i, 1)).toString();
        QString category = data(index(i, 3)).toString();
        int quantity = data(index(i, 4)).toInt();
        QString unit = data(index(i, 5)).toString();
        double price = data(index(i, 6)).toDouble();
        double value = quantity * price;
        totalValue += value;
        
        painter.drawText(margin, yPos, colWidth, rowHeight, Qt::AlignLeft | Qt::AlignVCenter, QString::number(id));
        painter.drawText(margin + colWidth, yPos, colWidth, rowHeight, Qt::AlignLeft | Qt::AlignVCenter, name);
        painter.drawText(margin + 2 * colWidth, yPos, colWidth, rowHeight, Qt::AlignLeft | Qt::AlignVCenter, category);
        painter.drawText(margin + 3 * colWidth, yPos, colWidth, rowHeight, Qt::AlignRight | Qt::AlignVCenter, QString::number(quantity));
        painter.drawText(margin + 4 * colWidth, yPos, colWidth, rowHeight, Qt::AlignLeft | Qt::AlignVCenter, unit);
        painter.drawText(margin + 5 * colWidth, yPos, colWidth, rowHeight, Qt::AlignRight | Qt::AlignVCenter, QString::number(price, 'f', 2));
        painter.drawText(margin + 6 * colWidth, yPos, colWidth, rowHeight, Qt::AlignRight | Qt::AlignVCenter, QString::number(value, 'f', 2));
        
        yPos += rowHeight;
    }
    
    // Draw total line
    yPos += 10;
    painter.drawLine(margin, yPos, pageWidth - margin, yPos);
    yPos += 20;
    
    painter.setFont(headerFont);
    painter.drawText(margin, yPos, pageWidth - 2 * margin - colWidth, rowHeight, Qt::AlignRight | Qt::AlignVCenter, tr("Total Value:"));
    painter.drawText(pageWidth - margin - colWidth, yPos, colWidth, rowHeight, Qt::AlignRight | Qt::AlignVCenter, QString::number(totalValue, 'f', 2));
    
    // End painting
    painter.end();
    
    // Restore filter
    setFilter(oldFilter);
    select();
    
    errorMessage = tr("%1 materials exported to PDF successfully").arg(rowCount);
    return true;
}