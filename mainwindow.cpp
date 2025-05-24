#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QProcess>
#include <QInputDialog>
#include <QtCharts>

QT_BEGIN_NAMESPACE
namespace QtCharts { }
QT_END_NAMESPACE

using namespace QtCharts;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_materialModel(nullptr)
    , m_quantityChartView(nullptr)
    , m_valueChartView(nullptr)
    , m_trendChartView(nullptr)
    , m_predictionChartView(nullptr)
    , m_priceRangeLabel(nullptr)
    , m_quantityRangeLabel(nullptr)
    , m_priceMinSlider(nullptr)
    , m_priceMaxSlider(nullptr)
    , m_quantityMinSlider(nullptr)
    , m_quantityMaxSlider(nullptr)
    , m_dateEditFrom(nullptr)
    , m_dateEditTo(nullptr)
    , m_lowStockCheckBox(nullptr)
    , m_statusLabel(nullptr)
    , m_totalItemsLabel(nullptr)
    , m_totalValueLabel(nullptr)
{
    ui->setupUi(this);
    
    // Setup database and model
    setupDatabase();
    setupMaterialModel();
    
    // Setup advanced filters
    setupAdvancedFilters();
    
    // Setup charts
    setupCharts();
    
    // Setup navigation
    setupNavigation();
    
    // Setup file operations toolbar
    setupFileOperationsToolbar();
    
    // Setup accessibility features
    setupAccessibility();
    
    // Connect signals/slots that aren't auto-connected
    connect(ui->comboBoxMaterialCategory, &QComboBox::currentTextChanged,
            this, [this](const QString &) { updateCategoryComboBox(); });
    
    // Initial UI setup
    clearForm();
    ui->splitter->setSizes(QList<int>() << 300 << 200);
    
    // Set window title
    setWindowTitle(tr("Material Management System"));
    
    // Ensure menu bar is visible and properly styled
    menuBar()->setVisible(true);
    menuBar()->setNativeMenuBar(false); // Force non-native menu bar for better compatibility
    
    // Set initial page
    ui->stackedWidget->setCurrentIndex(0);
    ui->btnMaterials->setChecked(true);
    
    // Setup keyboard shortcuts
    setupShortcuts();
    
    // Setup barcode scanner
    setupBarcodeScanner();
}

MainWindow::~MainWindow()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    
    delete m_materialModel;
    delete ui;
}

void MainWindow::setupDatabase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("materials.db");
    
    if (!m_db.open()) {
        QMessageBox::critical(this, tr("Database Error"),
                             tr("Could not open database: %1").arg(m_db.lastError().text()));
        return;
    }
}

void MainWindow::setupMaterialModel()
{
    m_materialModel = new MaterialModel(this, m_db);
    
    if (!m_materialModel->initialize()) {
        QMessageBox::warning(this, tr("Model Initialization Error"),
                           tr("Failed to initialize the material model."));
        return;
    }
    
    ui->tableViewMaterials->setModel(m_materialModel);
    
    // Hide the ID column
    ui->tableViewMaterials->hideColumn(0);
    
    // Resize columns to content
    ui->tableViewMaterials->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // Update category filter combobox
    updateCategoryComboBox();
}

void MainWindow::updateCategoryComboBox()
{
    // Remember current selection
    QString currentCategory = ui->comboBoxCategory->currentText();
    
    // Clear and repopulate
    ui->comboBoxCategory->clear();
    ui->comboBoxCategory->addItem(tr("All Categories"));
    
    QStringList categories = m_materialModel->getCategories();
    ui->comboBoxCategory->addItems(categories);
    
    // Also update the material form category combobox
    QString currentMaterialCategory = ui->comboBoxMaterialCategory->currentText();
    ui->comboBoxMaterialCategory->clear();
    ui->comboBoxMaterialCategory->addItems(categories);
    
    // Restore selections if possible
    int index = ui->comboBoxCategory->findText(currentCategory);
    if (index >= 0) {
        ui->comboBoxCategory->setCurrentIndex(index);
    }
    
    index = ui->comboBoxMaterialCategory->findText(currentMaterialCategory);
    if (index >= 0) {
        ui->comboBoxMaterialCategory->setCurrentIndex(index);
    }
}

void MainWindow::clearForm()
{
    ui->lineEditId->clear();
    ui->lineEditName->clear();
    ui->textEditDescription->clear();
    ui->spinBoxQuantity->setValue(0);
    ui->doubleSpinBoxPrice->setValue(0.0);
    ui->comboBoxUnit->setCurrentIndex(0);
    
    // Enable form for new entry
    ui->lineEditName->setFocus();
    ui->pushButtonSave->setEnabled(true);
    ui->pushButtonDelete->setEnabled(false);
}

void MainWindow::loadMaterialDetails(int materialId)
{
    // Find the material in the model
    for (int i = 0; i < m_materialModel->rowCount(); ++i) {
        if (m_materialModel->data(m_materialModel->index(i, 0)).toInt() == materialId) {
            // Load data into form
            ui->lineEditId->setText(QString::number(materialId));
            ui->lineEditName->setText(m_materialModel->data(m_materialModel->index(i, 1)).toString());
            ui->textEditDescription->setText(m_materialModel->data(m_materialModel->index(i, 2)).toString());
            
            QString category = m_materialModel->data(m_materialModel->index(i, 3)).toString();
            int categoryIndex = ui->comboBoxMaterialCategory->findText(category);
            if (categoryIndex >= 0) {
                ui->comboBoxMaterialCategory->setCurrentIndex(categoryIndex);
            }
            
            ui->spinBoxQuantity->setValue(m_materialModel->data(m_materialModel->index(i, 4)).toInt());
            
            QString unit = m_materialModel->data(m_materialModel->index(i, 5)).toString();
            int unitIndex = ui->comboBoxUnit->findText(unit);
            if (unitIndex >= 0) {
                ui->comboBoxUnit->setCurrentIndex(unitIndex);
            }
            
            ui->doubleSpinBoxPrice->setValue(m_materialModel->data(m_materialModel->index(i, 6)).toDouble());
            
            // Enable buttons
            ui->pushButtonSave->setEnabled(true);
            ui->pushButtonDelete->setEnabled(true);
            
            return;
        }
    }
    
    // If we get here, the material wasn't found
    QMessageBox::warning(this, tr("Material Not Found"),
                       tr("The selected material could not be found."));
    clearForm();
}

// Slot implementations
void MainWindow::on_tableViewMaterials_clicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    
    // Get the material ID from the first column of the selected row
    int row = index.row();
    int materialId = m_materialModel->data(m_materialModel->index(row, 0)).toInt();
    loadMaterialDetails(materialId);
}

void MainWindow::on_pushButtonNew_clicked()
{
    clearForm();
}

void MainWindow::on_pushButtonSave_clicked()
{
    QString name = ui->lineEditName->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"),
                           tr("Material name cannot be empty."));
        ui->lineEditName->setFocus();
        return;
    }
    
    QString description = ui->textEditDescription->toPlainText().trimmed();
    QString category = ui->comboBoxMaterialCategory->currentText();
    int quantity = ui->spinBoxQuantity->value();
    QString unit = ui->comboBoxUnit->currentText();
    double price = ui->doubleSpinBoxPrice->value();
    
    bool success;
    if (ui->lineEditId->text().isEmpty()) {
        // Add new material
        success = m_materialModel->addMaterial(name, description, category, quantity, unit, price);
        if (success) {
            QMessageBox::information(this, tr("Success"),
                                   tr("Material added successfully."));
            clearForm();
        }
    } else {
        // Update existing material
        int originalId = -1;
        int newId = ui->lineEditId->text().toInt();
        
        // Find the original ID from the selected row
        QModelIndex currentIndex = ui->tableViewMaterials->currentIndex();
        if (currentIndex.isValid()) {
            int row = currentIndex.row();
            originalId = m_materialModel->data(m_materialModel->index(row, 0)).toInt();
        } else {
            // If no row is selected, use the ID from the form as both original and new ID
            originalId = newId;
        }
        
        success = m_materialModel->updateMaterial(originalId, newId, name, description, category, quantity, unit, price);
        if (success) {
            QMessageBox::information(this, tr("Success"),
                                   tr("Material updated successfully."));
        }
    }
    
    if (!success) {
        QMessageBox::warning(this, tr("Database Error"),
                           tr("Failed to save material."));
    }
    
    // Update category combobox in case a new category was added
    updateCategoryComboBox();
    
    // Update charts with new data
    updateCharts();
}

void MainWindow::on_pushButtonDelete_clicked()
{
    if (ui->lineEditId->text().isEmpty()) {
        return;
    }
    
    int id = ui->lineEditId->text().toInt();
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Confirm Deletion"),
        tr("Are you sure you want to delete this material?"),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        bool success = m_materialModel->deleteMaterial(id);
        if (success) {
            QMessageBox::information(this, tr("Success"),
                                   tr("Material deleted successfully."));
            clearForm();
            
            // Update charts after deletion
            updateCharts();
        } else {
            QMessageBox::warning(this, tr("Database Error"),
                               tr("Failed to delete material."));
        }
    }
}

void MainWindow::on_pushButtonClearFilters_clicked()
{
    ui->lineEditSearch->clear();
    ui->comboBoxCategory->setCurrentIndex(0);
    m_materialModel->clearFilters();
}

void MainWindow::on_lineEditSearch_textChanged(const QString &text)
{
    QString category = ui->comboBoxCategory->currentText();
    if (category == tr("All Categories")) {
        category = "";
    }
    
    m_materialModel->filterMaterials(text, category);
}

void MainWindow::on_comboBoxCategory_currentTextChanged(const QString &text)
{
    QString searchTerm = ui->lineEditSearch->text();
    QString category = text;
    
    if (category == tr("All Categories")) {
        category = "";
    }
    
    m_materialModel->filterMaterials(searchTerm, category);
}

void MainWindow::on_actionNewMaterial_triggered()
{
    clearForm();
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionImportMaterials_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Materials"),
                                                 "", tr("CSV Files (*.csv)"));
    if (fileName.isEmpty()) {
        return;
    }
    
    QString errorMessage;
    bool success = m_materialModel->importFromCSV(fileName, errorMessage);
    
    if (success) {
        QMessageBox::information(this, tr("Import Complete"), errorMessage);
        // Update category combobox
        updateCategoryComboBox();
        // Update charts with new data
        updateCharts();
    } else {
        QMessageBox::warning(this, tr("Import Error"), errorMessage);
    }
}

void MainWindow::on_actionExportMaterials_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Materials"),
                                                 "", tr("PDF Files (*.pdf)"));
    if (fileName.isEmpty()) {
        return;
    }
    
    // Add .pdf extension if not present
    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
        fileName += ".pdf";
    }
    
    QString errorMessage;
    bool success = m_materialModel->exportToPDF(fileName, errorMessage);
    
    if (success) {
        QMessageBox::information(this, tr("Export Complete"), errorMessage);
    } else {
        QMessageBox::warning(this, tr("Export Error"), errorMessage);
    }
}

// Chart methods
void MainWindow::setupCharts()
{
    // Create chart views if they don't exist
    if (!m_quantityChartView) {
        // Create quantity chart
        QChart *quantityChart = new QChart();
        quantityChart->setTitle(tr("Material Quantities by Category"));
        quantityChart->setAnimationOptions(QChart::SeriesAnimations);
        quantityChart->setTheme(QChart::ChartThemeDark);
        
        m_quantityChartView = new QChartView(quantityChart);
        m_quantityChartView->setRenderHint(QPainter::Antialiasing);
        ui->chartQuantityLayout->addWidget(m_quantityChartView);
    }
    
    if (!m_valueChartView) {
        // Create value chart
        QChart *valueChart = new QChart();
        valueChart->setTitle(tr("Material Values by Category"));
        valueChart->setAnimationOptions(QChart::SeriesAnimations);
        valueChart->setTheme(QChart::ChartThemeDark);
        
        m_valueChartView = new QChartView(valueChart);
        m_valueChartView->setRenderHint(QPainter::Antialiasing);
        ui->chartValueLayout->addWidget(m_valueChartView);
    }
    
    // Update charts with data
    updateCharts();
}

void MainWindow::updateCharts()
{
    // Get data from model
    QList<QPair<QString, int>> quantityData = m_materialModel->getMaterialQuantitiesForChart();
    QList<QPair<QString, double>> valueData = m_materialModel->getMaterialValuesForChart();
    
    // Define color scheme based on UI specifications
    QList<QColor> colorScheme = {
        QColor("#E3C6B0"), // Primary Light
        QColor("#D4B7A1"), // Accent Light
        QColor("#3D485A"), // Primary Dark
        QColor("#2A3340"), // Accent Dark
        QColor("#4CAF50"), // Success
        QColor("#FFA726"), // Warning
        QColor("#FF6B6B")  // Error
    };
    
    // Calculate total quantities and values for statistics
    int totalQuantity = 0;
    double totalValue = 0.0;
    for (const auto &pair : quantityData) {
        totalQuantity += pair.second;
    }
    for (const auto &pair : valueData) {
        totalValue += pair.second;
    }
    
    // Create quantity chart with enhanced visuals
    QChart *quantityChart = new QChart();
    quantityChart->setTitle(tr("Material Quantities by Category"));
    quantityChart->setAnimationOptions(QChart::SeriesAnimations | QChart::GridAxisAnimations);
    quantityChart->setBackgroundVisible(false);
    quantityChart->setBackgroundRoundness(8);
    quantityChart->setTitleFont(QFont("Segoe UI", 12, QFont::Bold));
    quantityChart->setTitleBrush(QBrush(QColor("#E3C6B0"))); // Primary Light
    
    // Create interactive quantity series
    QPieSeries *quantitySeries = new QPieSeries();
    int colorIndex = 0;
    for (const auto &pair : quantityData) {
        // Calculate percentage
        double percentage = (totalQuantity > 0) ? (pair.second * 100.0 / totalQuantity) : 0;
        QString label = QString("%1\n%2 (%3%)")
                        .arg(pair.first)
                        .arg(pair.second)
                        .arg(percentage, 0, 'f', 1);
        
        QPieSlice *slice = quantitySeries->append(label, pair.second);
        slice->setLabelVisible(true);
        slice->setLabelColor(QColor("#FFFFFF"));
        slice->setLabelPosition(QPieSlice::LabelOutside);
        slice->setBrush(colorScheme[colorIndex % colorScheme.size()]);
        slice->setPen(QPen(Qt::white, 1));
        
        // Connect signals for interactive hover effects
        connect(slice, &QPieSlice::hovered, [slice](bool hovered) {
            if (hovered) {
                slice->setExploded(true);
                slice->setLabelFont(QFont("Segoe UI", 10, QFont::Bold));
            } else {
                slice->setExploded(false);
                slice->setLabelFont(QFont("Segoe UI", 9));
            }
        });
        
        colorIndex++;
    }
    
    // Add series to chart
    quantityChart->addSeries(quantitySeries);
    quantityChart->legend()->setAlignment(Qt::AlignRight);
    quantityChart->legend()->setLabelColor(QColor("#FFFFFF"));
    quantityChart->legend()->setFont(QFont("Segoe UI", 9));
    quantityChart->legend()->setMarkerShape(QLegend::MarkerShapeCircle);
    
    // Add statistical information to quantity chart
    QLabel *quantityStatsLabel = new QLabel();
    quantityStatsLabel->setText(tr("Total Quantity: %1 items").arg(totalQuantity));
    quantityStatsLabel->setStyleSheet("color: #E3C6B0; font-weight: bold; background-color: transparent;");
    quantityStatsLabel->setAlignment(Qt::AlignCenter);
    ui->chartQuantityLayout->addWidget(quantityStatsLabel);
    quantityStatsLabel->show();
    
    // Create value distribution chart with enhanced visuals
    QChart *valueChart = new QChart();
    valueChart->setTitle(tr("Material Values by Category"));
    valueChart->setAnimationOptions(QChart::SeriesAnimations | QChart::GridAxisAnimations);
    valueChart->setBackgroundVisible(false);
    valueChart->setBackgroundRoundness(8);
    valueChart->setTitleFont(QFont("Segoe UI", 12, QFont::Bold));
    valueChart->setTitleBrush(QBrush(QColor("#E3C6B0"))); // Primary Light
    
    // Create interactive value series
    QPieSeries *valueSeries = new QPieSeries();
    colorIndex = 0;
    for (const auto &pair : valueData) {
        // Calculate percentage
        double percentage = (totalValue > 0) ? (pair.second * 100.0 / totalValue) : 0;
        QString label = QString("%1\n$%2 (%3%)")
                        .arg(pair.first)
                        .arg(pair.second, 0, 'f', 2)
                        .arg(percentage, 0, 'f', 1);
        
        QPieSlice *slice = valueSeries->append(label, pair.second);
        slice->setLabelVisible(true);
        slice->setLabelColor(QColor("#FFFFFF"));
        slice->setLabelPosition(QPieSlice::LabelOutside);
        slice->setBrush(colorScheme[colorIndex % colorScheme.size()]);
        slice->setPen(QPen(Qt::white, 1));
        
        // Connect signals for interactive hover effects
        connect(slice, &QPieSlice::hovered, [slice](bool hovered) {
            if (hovered) {
                slice->setExploded(true);
                slice->setLabelFont(QFont("Segoe UI", 10, QFont::Bold));
            } else {
                slice->setExploded(false);
                slice->setLabelFont(QFont("Segoe UI", 9));
            }
        });
        
        colorIndex++;
    }
    
    // Add series to chart
    valueChart->addSeries(valueSeries);
    valueChart->legend()->setAlignment(Qt::AlignRight);
    valueChart->legend()->setLabelColor(QColor("#FFFFFF"));
    valueChart->legend()->setFont(QFont("Segoe UI", 9));
    valueChart->legend()->setMarkerShape(QLegend::MarkerShapeCircle);
    
    // Add statistical information to value chart
    QLabel *valueStatsLabel = new QLabel();
    valueStatsLabel->setText(tr("Total Value: $%1").arg(totalValue, 0, 'f', 2));
    valueStatsLabel->setStyleSheet("color: #E3C6B0; font-weight: bold; background-color: transparent;");
    valueStatsLabel->setAlignment(Qt::AlignCenter);
    ui->chartValueLayout->addWidget(valueStatsLabel);
    valueStatsLabel->show();
    
    // Take ownership of old charts (if any) to delete them later
    QChart *oldQuantityChart = m_quantityChartView->chart();
    QChart *oldValueChart = m_valueChartView->chart();
    
    // Set new charts
    m_quantityChartView->setChart(quantityChart);
    m_valueChartView->setChart(valueChart);
    m_quantityChartView->setRenderHint(QPainter::Antialiasing);
    m_valueChartView->setRenderHint(QPainter::Antialiasing);
    
    // Enable chart interactions
    m_quantityChartView->setRubberBand(QChartView::RectangleRubberBand);
    m_valueChartView->setRubberBand(QChartView::RectangleRubberBand);
    
    // Delete old charts
    if (oldQuantityChart && oldQuantityChart != quantityChart) {
        delete oldQuantityChart;
    }
    
    if (oldValueChart && oldValueChart != valueChart) {
        delete oldValueChart;
    }
}

// Implementation of advanced filtering setup
void MainWindow::setupAdvancedFilters()
{
    // Create filter container widget since it doesn't exist in UI
    QWidget* filterContainer = new QWidget(this);
    filterContainer->setObjectName("filterContainer");
    
    // Create price range filter
    QGroupBox* priceFilterGroup = new QGroupBox(tr("Price Range Filter"), filterContainer);
    priceFilterGroup->setObjectName("priceFilterGroup");
    QVBoxLayout* priceLayout = new QVBoxLayout(priceFilterGroup);
    
    m_priceRangeLabel = new QLabel(tr("Price: $0 - $1000"), priceFilterGroup);
    priceLayout->addWidget(m_priceRangeLabel);
    
    QHBoxLayout* priceSliderLayout = new QHBoxLayout();
    
    m_priceMinSlider = new QSlider(Qt::Horizontal, priceFilterGroup);
    m_priceMinSlider->setObjectName("sliderPriceMin");
    m_priceMinSlider->setRange(0, 1000);
    m_priceMinSlider->setValue(0);
    m_priceMinSlider->setTracking(true);
    
    m_priceMaxSlider = new QSlider(Qt::Horizontal, priceFilterGroup);
    m_priceMaxSlider->setObjectName("sliderPriceMax");
    m_priceMaxSlider->setRange(0, 1000);
    m_priceMaxSlider->setValue(1000);
    m_priceMaxSlider->setTracking(true);
    
    priceSliderLayout->addWidget(m_priceMinSlider);
    priceSliderLayout->addWidget(m_priceMaxSlider);
    priceLayout->addLayout(priceSliderLayout);
    
    // Create quantity range filter
    QGroupBox* quantityFilterGroup = new QGroupBox(tr("Quantity Range Filter"), filterContainer);
    quantityFilterGroup->setObjectName("quantityFilterGroup");
    QVBoxLayout* quantityLayout = new QVBoxLayout(quantityFilterGroup);
    
    m_quantityRangeLabel = new QLabel(tr("Quantity: 0 - 1000"), quantityFilterGroup);
    quantityLayout->addWidget(m_quantityRangeLabel);
    
    QHBoxLayout* quantitySliderLayout = new QHBoxLayout();
    
    m_quantityMinSlider = new QSlider(Qt::Horizontal, quantityFilterGroup);
    m_quantityMinSlider->setObjectName("sliderQuantityMin");
    m_quantityMinSlider->setRange(0, 1000);
    m_quantityMinSlider->setValue(0);
    m_quantityMinSlider->setTracking(true);
    
    m_quantityMaxSlider = new QSlider(Qt::Horizontal, quantityFilterGroup);
    m_quantityMaxSlider->setObjectName("sliderQuantityMax");
    m_quantityMaxSlider->setRange(0, 1000);
    m_quantityMaxSlider->setValue(1000);
    m_quantityMaxSlider->setTracking(true);
    
    quantitySliderLayout->addWidget(m_quantityMinSlider);
    quantitySliderLayout->addWidget(m_quantityMaxSlider);
    quantityLayout->addLayout(quantitySliderLayout);
    
    // Date range filter
    QGroupBox* dateFilterGroup = new QGroupBox(tr("Date Range Filter"), filterContainer);
    dateFilterGroup->setObjectName("dateFilterGroup");
    QVBoxLayout* dateLayout = new QVBoxLayout(dateFilterGroup);
    
    QLabel* dateRangeLabel = new QLabel(tr("Date Range:"), dateFilterGroup);
    dateLayout->addWidget(dateRangeLabel);
    
    QHBoxLayout* dateEditLayout = new QHBoxLayout();
    
    m_dateEditFrom = new QDateEdit(QDate::currentDate().addMonths(-1), dateFilterGroup);
    m_dateEditFrom->setObjectName("dateEditFrom");
    m_dateEditFrom->setCalendarPopup(true);
    
    m_dateEditTo = new QDateEdit(QDate::currentDate(), dateFilterGroup);
    m_dateEditTo->setObjectName("dateEditTo");
    m_dateEditTo->setCalendarPopup(true);
    
    dateEditLayout->addWidget(new QLabel(tr("From:")));
    dateEditLayout->addWidget(m_dateEditFrom);
    dateEditLayout->addWidget(new QLabel(tr("To:")));
    dateEditLayout->addWidget(m_dateEditTo);
    dateLayout->addLayout(dateEditLayout);
    
    // Low stock indicator
    m_lowStockCheckBox = new QCheckBox(tr("Show Low Stock Items Only"), filterContainer);
    m_lowStockCheckBox->setObjectName("checkBoxLowStock");
    QSpinBox* lowStockThresholdSpinBox = new QSpinBox(filterContainer);
    lowStockThresholdSpinBox->setObjectName("lowStockThresholdSpinBox");
    lowStockThresholdSpinBox->setMinimum(1);
    lowStockThresholdSpinBox->setMaximum(100);
    lowStockThresholdSpinBox->setValue(10); // Default threshold
    lowStockThresholdSpinBox->setToolTip(tr("Items with quantity below this value will be considered low stock"));
    
    // Connect the threshold change to check for low stock items
    connect(lowStockThresholdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::checkLowStockItems);
    
    // Add all filter groups to the container
    QVBoxLayout* filterLayout = new QVBoxLayout(filterContainer);
    filterLayout->addWidget(priceFilterGroup);
    filterLayout->addWidget(quantityFilterGroup);
    filterLayout->addWidget(dateFilterGroup);
    filterLayout->addWidget(m_lowStockCheckBox);
    filterLayout->addStretch();
    
    // Status indicators - create a container since it doesn't exist in UI
    QWidget* statusContainer = new QWidget(this);
    statusContainer->setObjectName("statusContainer");
    QHBoxLayout* statusLayout = new QHBoxLayout(statusContainer);
    
    m_totalItemsLabel = new QLabel(tr("Total Items: 0"), statusContainer);
    m_totalItemsLabel->setStyleSheet("color: #E3C6B0; font-weight: bold;");
    
    m_totalValueLabel = new QLabel(tr("Total Value: $0.00"), statusContainer);
    m_totalValueLabel->setStyleSheet("color: #E3C6B0; font-weight: bold;");
    
    m_statusLabel = new QLabel("", statusContainer);
    
    statusLayout->addWidget(m_totalItemsLabel);
    statusLayout->addWidget(m_totalValueLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(m_statusLabel);
    
    // Connect signals for filter changes
    connect(m_priceMinSlider, &QSlider::valueChanged, this, &MainWindow::on_sliderPriceMin_valueChanged);
    connect(m_priceMaxSlider, &QSlider::valueChanged, this, &MainWindow::on_sliderPriceMax_valueChanged);
    connect(m_quantityMinSlider, &QSlider::valueChanged, this, &MainWindow::on_sliderQuantityMin_valueChanged);
    connect(m_quantityMaxSlider, &QSlider::valueChanged, this, &MainWindow::on_sliderQuantityMax_valueChanged);
    connect(m_dateEditFrom, &QDateEdit::dateChanged, this, &MainWindow::on_dateEditFrom_dateChanged);
    connect(m_dateEditTo, &QDateEdit::dateChanged, this, &MainWindow::on_dateEditTo_dateChanged);
    connect(m_lowStockCheckBox, &QCheckBox::toggled, this, &MainWindow::on_checkBoxLowStock_toggled);
}

// Implementation of navigation setup
void MainWindow::setupNavigation()
{
    // Connect sidebar navigation buttons
    connect(ui->btnMaterials, &QPushButton::clicked, this, &MainWindow::on_btnMaterials_clicked);
    connect(ui->btnDashboard, &QPushButton::clicked, this, &MainWindow::on_btnDashboard_clicked);
    
    // Create additional navigation buttons if they don't exist
    QWidget* sidebarWidget = ui->sidebarWidget;
    QVBoxLayout* sidebarLayout = qobject_cast<QVBoxLayout*>(sidebarWidget->layout());
    
    if (sidebarLayout) {
        // Add Suppliers button
        QPushButton* btnSuppliers = new QPushButton(tr("  Suppliers"), sidebarWidget);
        btnSuppliers->setObjectName("btnSuppliers");
        btnSuppliers->setCheckable(true);
        btnSuppliers->setMinimumHeight(50);
        btnSuppliers->setIcon(QIcon(":/icons/suppliers.svg"));
        btnSuppliers->setIconSize(QSize(24, 24));
        connect(btnSuppliers, &QPushButton::clicked, this, &MainWindow::on_btnSuppliers_clicked);
        
        // Add Users button
        QPushButton* btnUsers = new QPushButton(tr("  Users"), sidebarWidget);
        btnUsers->setObjectName("btnUsers");
        btnUsers->setCheckable(true);
        btnUsers->setMinimumHeight(50);
        btnUsers->setIcon(QIcon(":/icons/users.svg"));
        btnUsers->setIconSize(QSize(24, 24));
        connect(btnUsers, &QPushButton::clicked, this, &MainWindow::on_btnUsers_clicked);
        
        // Add Reports button
        QPushButton* btnReports = new QPushButton(tr("  Reports"), sidebarWidget);
        btnReports->setObjectName("btnReports");
        btnReports->setCheckable(true);
        btnReports->setMinimumHeight(50);
        btnReports->setIcon(QIcon(":/icons/reports.svg"));
        btnReports->setIconSize(QSize(24, 24));
        connect(btnReports, &QPushButton::clicked, this, &MainWindow::on_btnReports_clicked);
        
        // Add Settings button
        QPushButton* btnSettings = new QPushButton(tr("  Settings"), sidebarWidget);
        btnSettings->setObjectName("btnSettings");
        btnSettings->setCheckable(true);
        btnSettings->setMinimumHeight(50);
        btnSettings->setIcon(QIcon(":/icons/settings.svg"));
        btnSettings->setIconSize(QSize(24, 24));
        connect(btnSettings, &QPushButton::clicked, this, &MainWindow::on_btnSettings_clicked);
        
        // Insert new buttons after Dashboard but before the spacer
        int dashboardIndex = sidebarLayout->indexOf(ui->btnDashboard);
        if (dashboardIndex >= 0) {
            sidebarLayout->insertWidget(dashboardIndex + 1, btnSuppliers);
            sidebarLayout->insertWidget(dashboardIndex + 2, btnUsers);
            sidebarLayout->insertWidget(dashboardIndex + 3, btnReports);
            sidebarLayout->insertWidget(dashboardIndex + 4, btnSettings);
        }
    }
}

// Implementation of accessibility setup
void MainWindow::setupAccessibility()
{
    // Set focus policy for all interactive elements
    ui->tableViewMaterials->setFocusPolicy(Qt::StrongFocus);
    ui->lineEditSearch->setFocusPolicy(Qt::StrongFocus);
    ui->comboBoxCategory->setFocusPolicy(Qt::StrongFocus);
    
    // Set tab order for keyboard navigation
    setTabOrder(ui->lineEditSearch, ui->comboBoxCategory);
    setTabOrder(ui->comboBoxCategory, ui->tableViewMaterials);
    setTabOrder(ui->tableViewMaterials, ui->lineEditName);
    
    // Add accessibility descriptions
    ui->lineEditSearch->setAccessibleName(tr("Search materials"));
    ui->lineEditSearch->setAccessibleDescription(tr("Enter text to search for materials by name or description"));
    
    ui->comboBoxCategory->setAccessibleName(tr("Filter by category"));
    ui->comboBoxCategory->setAccessibleDescription(tr("Select a category to filter the materials list"));
    
    ui->tableViewMaterials->setAccessibleName(tr("Materials list"));
    ui->tableViewMaterials->setAccessibleDescription(tr("Table showing all materials matching the current filters"));
    
    // High contrast focus indicators
    QString focusStyle = "*:focus { border: 2px solid #E3C6B0; }"; // Primary Light
    setStyleSheet(styleSheet() + focusStyle);
}

void MainWindow::setupShortcuts()
{
    // Material management shortcuts
    QShortcut *newShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this);
    connect(newShortcut, &QShortcut::activated, this, &MainWindow::on_pushButtonNew_clicked);
    ui->pushButtonNew->setToolTip(tr("New Material (Ctrl+N)"));
    
    QShortcut *saveShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this);
    connect(saveShortcut, &QShortcut::activated, this, &MainWindow::on_pushButtonSave_clicked);
    ui->pushButtonSave->setToolTip(tr("Save Material (Ctrl+S)"));
    
    QShortcut *deleteShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_D), this);
    connect(deleteShortcut, &QShortcut::activated, this, &MainWindow::on_pushButtonDelete_clicked);
    ui->pushButtonDelete->setToolTip(tr("Delete Material (Ctrl+D)"));
    
    QShortcut *clearFiltersShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this);
    connect(clearFiltersShortcut, &QShortcut::activated, this, &MainWindow::on_pushButtonClearFilters_clicked);
    ui->pushButtonClearFilters->setToolTip(tr("Clear Filters (Ctrl+R)"));
    
    // Navigation shortcuts
    QShortcut *dashboardShortcut = new QShortcut(QKeySequence(Qt::ALT | Qt::Key_1), this);
    connect(dashboardShortcut, &QShortcut::activated, this, &MainWindow::on_btnDashboard_clicked);
    ui->btnDashboard->setToolTip(tr("Dashboard (Alt+1)"));
    
    QShortcut *materialsShortcut = new QShortcut(QKeySequence(Qt::ALT | Qt::Key_2), this);
    connect(materialsShortcut, &QShortcut::activated, this, &MainWindow::on_btnMaterials_clicked);
    ui->btnMaterials->setToolTip(tr("Materials (Alt+2)"));
    
    QShortcut *suppliersShortcut = new QShortcut(QKeySequence(Qt::ALT | Qt::Key_3), this);
    connect(suppliersShortcut, &QShortcut::activated, this, &MainWindow::on_btnSuppliers_clicked);
    // Find the dynamically created button and set its tooltip
    QPushButton *btnSuppliers = findChild<QPushButton*>("btnSuppliers");
    if (btnSuppliers) {
        btnSuppliers->setToolTip(tr("Suppliers (Alt+3)"));
    }
    
    QShortcut *usersShortcut = new QShortcut(QKeySequence(Qt::ALT | Qt::Key_4), this);
    connect(usersShortcut, &QShortcut::activated, this, &MainWindow::on_btnUsers_clicked);
    // Find the dynamically created button and set its tooltip
    QPushButton *btnUsers = findChild<QPushButton*>("btnUsers");
    if (btnUsers) {
        btnUsers->setToolTip(tr("Users (Alt+4)"));
    }
    
    QShortcut *reportsShortcut = new QShortcut(QKeySequence(Qt::ALT | Qt::Key_5), this);
    connect(reportsShortcut, &QShortcut::activated, this, &MainWindow::on_btnReports_clicked);
    // Find the dynamically created button and set its tooltip
    QPushButton *btnReports = findChild<QPushButton*>("btnReports");
    if (btnReports) {
        btnReports->setToolTip(tr("Reports (Alt+5)"));
    }
    
    QShortcut *settingsShortcut = new QShortcut(QKeySequence(Qt::ALT | Qt::Key_6), this);
    connect(settingsShortcut, &QShortcut::activated, this, &MainWindow::on_btnSettings_clicked);
    // Find the dynamically created button and set its tooltip
    QPushButton *btnSettings = findChild<QPushButton*>("btnSettings");
    if (btnSettings) {
        btnSettings->setToolTip(tr("Settings (Alt+6)"));
    }
    
    // File operations shortcuts
    QShortcut *importShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_I), this);
    connect(importShortcut, &QShortcut::activated, this, &MainWindow::on_actionImportMaterials_triggered);
    ui->actionImportMaterials->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    
    QShortcut *exportShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_E), this);
    connect(exportShortcut, &QShortcut::activated, this, &MainWindow::on_actionExportMaterials_triggered);
    ui->actionExportMaterials->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    
    // Additional shortcuts
    QShortcut *searchFocusShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    connect(searchFocusShortcut, &QShortcut::activated, this, [this](){ ui->lineEditSearch->setFocus(); });
    ui->lineEditSearch->setToolTip(tr("Search (Ctrl+F)"));
    
    QShortcut *exitShortcut = new QShortcut(QKeySequence(Qt::ALT | Qt::Key_F4), this);
    connect(exitShortcut, &QShortcut::activated, this, &MainWindow::close);
    ui->actionExit->setShortcut(QKeySequence(Qt::ALT | Qt::Key_F4));
}

void MainWindow::setupFileOperationsToolbar()
{
    // Create a toolbar for file operations
    QToolBar *fileToolbar = new QToolBar(tr("File Operations"), this);
    fileToolbar->setObjectName("fileOperationsToolbar");
    fileToolbar->setMovable(false);
    fileToolbar->setIconSize(QSize(24, 24));
    
    // Create import button with dropdown menu
    QToolButton *importButton = new QToolButton(this);
    importButton->setText(tr("Import"));
    importButton->setIcon(QIcon::fromTheme("document-import", QIcon(":/icons/import.svg")));
    importButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    importButton->setPopupMode(QToolButton::InstantPopup);
    
    // Create import menu
    QMenu *importMenu = new QMenu(this);
    QAction *importCSVAction = importMenu->addAction(tr("Import from CSV..."));
    connect(importCSVAction, &QAction::triggered, this, &MainWindow::on_actionImportMaterials_triggered);
    importButton->setMenu(importMenu);
    
    // Create export button with dropdown menu
    QToolButton *exportButton = new QToolButton(this);
    exportButton->setText(tr("Export"));
    exportButton->setIcon(QIcon::fromTheme("document-export", QIcon(":/icons/export.svg")));
    exportButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    exportButton->setPopupMode(QToolButton::InstantPopup);
    
    // Create export menu
    QMenu *exportMenu = new QMenu(this);
    QAction *exportPDFAction = exportMenu->addAction(tr("Export to PDF..."));
    connect(exportPDFAction, &QAction::triggered, this, &MainWindow::on_actionExportMaterials_triggered);
    exportButton->setMenu(exportMenu);
    
    // Add buttons to toolbar
    fileToolbar->addWidget(importButton);
    fileToolbar->addWidget(exportButton);
    
    // Add toolbar to main window
    addToolBar(Qt::TopToolBarArea, fileToolbar);
    
    // Set accessibility properties
    importButton->setAccessibleName(tr("Import Materials Button"));
    exportButton->setAccessibleName(tr("Export Materials Button"));
    
    // Apply stylesheet to match the application theme
    QString toolbarStyle = "QToolBar { background-color: #3D485A; border-bottom: 1px solid #2A3340; spacing: 5px; } "
                          "QToolButton { background-color: #E3C6B0; color: #2A3340; border: none; padding: 5px 10px; border-radius: 4px; } "
                          "QToolButton:hover { background-color: #D4B7A1; } "
                          "QToolButton::menu-indicator { image: none; } "
                          "QMenuBar { background-color: #3D485A; color: #FFFFFF; border-bottom: 1px solid #2A3340; padding: 2px; } "
                          "QMenuBar::item { background-color: transparent; padding: 5px 10px; } "
                          "QMenuBar::item:selected { background-color: #2A3340; color: #E3C6B0; }"; // Hide the default menu indicator
    fileToolbar->setStyleSheet(toolbarStyle);
    
    // Apply the same style to the entire application to ensure menu visibility
    this->setStyleSheet(toolbarStyle);
}

// Navigation slot implementations
void MainWindow::on_btnMaterials_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->btnMaterials->setChecked(true);
    
    // Uncheck other buttons
    ui->btnDashboard->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnSuppliers"))
        ui->sidebarWidget->findChild<QPushButton*>("btnSuppliers")->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnUsers"))
        ui->sidebarWidget->findChild<QPushButton*>("btnUsers")->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnReports"))
        ui->sidebarWidget->findChild<QPushButton*>("btnReports")->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnSettings"))
        ui->sidebarWidget->findChild<QPushButton*>("btnSettings")->setChecked(false);
}

void MainWindow::on_btnDashboard_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->btnDashboard->setChecked(true);
    
    // Uncheck other buttons
    ui->btnMaterials->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnSuppliers"))
        ui->sidebarWidget->findChild<QPushButton*>("btnSuppliers")->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnUsers"))
        ui->sidebarWidget->findChild<QPushButton*>("btnUsers")->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnReports"))
        ui->sidebarWidget->findChild<QPushButton*>("btnReports")->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnSettings"))
        ui->sidebarWidget->findChild<QPushButton*>("btnSettings")->setChecked(false);
    
    // Update charts when dashboard is shown
    updateCharts();
}

// Filter slot implementations
void MainWindow::on_sliderPriceMin_valueChanged(int value)
{
    // Ensure min doesn't exceed max
    if (value > m_priceMaxSlider->value()) {
        m_priceMinSlider->setValue(m_priceMaxSlider->value());
        return;
    }
    
    // Update label
    m_priceRangeLabel->setText(tr("Price: $%1 - $%2")
                              .arg(value)
                              .arg(m_priceMaxSlider->value()));
    
    // Apply filters
    applyFilters();
}

void MainWindow::on_sliderPriceMax_valueChanged(int value)
{
    // Ensure max doesn't go below min
    if (value < m_priceMinSlider->value()) {
        m_priceMaxSlider->setValue(m_priceMinSlider->value());
        return;
    }
    
    // Update label
    m_priceRangeLabel->setText(tr("Price: $%1 - $%2")
                              .arg(m_priceMinSlider->value())
                              .arg(value));
    
    // Apply filters
    applyFilters();
}

void MainWindow::on_sliderQuantityMin_valueChanged(int value)
{
    // Ensure min doesn't exceed max
    if (value > m_quantityMaxSlider->value()) {
        m_quantityMinSlider->setValue(m_quantityMaxSlider->value());
        return;
    }
    
    // Update label
    m_quantityRangeLabel->setText(tr("Quantity: %1 - %2")
                                 .arg(value)
                                 .arg(m_quantityMaxSlider->value()));
    
    // Apply filters
    applyFilters();
}

void MainWindow::on_sliderQuantityMax_valueChanged(int value)
{
    // Ensure max doesn't go below min
    if (value < m_quantityMinSlider->value()) {
        m_quantityMaxSlider->setValue(m_quantityMinSlider->value());
        return;
    }
    
    // Update label
    m_quantityRangeLabel->setText(tr("Quantity: %1 - %2")
                                 .arg(m_quantityMinSlider->value())
                                 .arg(value));
    
    // Apply filters
    applyFilters();
}

// Apply all filters to the material model
void MainWindow::applyFilters()
{
    QString searchTerm = ui->lineEditSearch->text();
    QString category = ui->comboBoxCategory->currentText();
    if (category == tr("All Categories")) {
        category = "";
    }
    
    int priceMin = m_priceMinSlider->value();
    int priceMax = m_priceMaxSlider->value();
    int quantityMin = m_quantityMinSlider->value();
    int quantityMax = m_quantityMaxSlider->value();
    
    // Use member variables for date range instead of finding them in the container
    QDate fromDate = m_dateEditFrom ? m_dateEditFrom->date() : QDate::currentDate();
    QDate toDate = m_dateEditTo ? m_dateEditTo->date() : QDate::currentDate();
    
    // Use member variable for low stock checkbox
    bool lowStockOnly = m_lowStockCheckBox ? m_lowStockCheckBox->isChecked() : false;
    
    // Apply advanced filters to model
    // Note: This would require extending the MaterialModel class with advanced filtering capabilities
    // For now, we'll just use the basic filtering
    m_materialModel->filterMaterials(searchTerm, category);
    
    // Update status indicators
    int totalItems = m_materialModel->rowCount();
    double totalValue = 0.0;
    
    for (int i = 0; i < totalItems; ++i) {
        double price = m_materialModel->data(m_materialModel->index(i, 6)).toDouble();
        int quantity = m_materialModel->data(m_materialModel->index(i, 4)).toInt();
        totalValue += price * quantity;
    }
    
    if (m_totalItemsLabel) {
        m_totalItemsLabel->setText(tr("Total Items: %1").arg(totalItems));
    }
    
    if (m_totalValueLabel) {
        m_totalValueLabel->setText(tr("Total Value: $%1").arg(totalValue, 0, 'f', 2));
    }
}

// Accessibility helper methods
void MainWindow::announceError(const QString &message)
{
    if (m_statusLabel) {
        m_statusLabel->setText(message);
        m_statusLabel->setStyleSheet("color: #FF6B6B; font-weight: bold;"); // Error color
    }
    
    // This would ideally use a screen reader API
    QApplication::beep();
}

void MainWindow::announceSuccess(const QString &message)
{
    if (m_statusLabel) {
        m_statusLabel->setText(message);
        m_statusLabel->setStyleSheet("color: #4CAF50; font-weight: bold;"); // Success color
    }
}

// Placeholder implementations for new navigation methods
void MainWindow::on_btnSuppliers_clicked()
{
    // Set the stackedWidget index
    ui->stackedWidget->setCurrentIndex(2);
    
    // Uncheck other buttons
    ui->btnMaterials->setChecked(false);
    ui->btnDashboard->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnUsers"))
        ui->sidebarWidget->findChild<QPushButton*>("btnUsers")->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnReports"))
        ui->sidebarWidget->findChild<QPushButton*>("btnReports")->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnSettings"))
        ui->sidebarWidget->findChild<QPushButton*>("btnSettings")->setChecked(false);
    
    // Set this button as checked
    QPushButton* btnSuppliers = ui->sidebarWidget->findChild<QPushButton*>("btnSuppliers");
    if (btnSuppliers) {
        btnSuppliers->setChecked(true);
    }
    
    // Load suppliers page (placeholder)
    loadSuppliersPage();
}

void MainWindow::on_btnUsers_clicked()
{
    // Set the stackedWidget index
    ui->stackedWidget->setCurrentIndex(3);
    
    // Uncheck other buttons
    ui->btnMaterials->setChecked(false);
    ui->btnDashboard->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnSuppliers"))
        ui->sidebarWidget->findChild<QPushButton*>("btnSuppliers")->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnReports"))
        ui->sidebarWidget->findChild<QPushButton*>("btnReports")->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnSettings"))
        ui->sidebarWidget->findChild<QPushButton*>("btnSettings")->setChecked(false);
    
    // Set this button as checked
    QPushButton* btnUsers = ui->sidebarWidget->findChild<QPushButton*>("btnUsers");
    if (btnUsers) {
        btnUsers->setChecked(true);
    }
    
    // Load users page (placeholder)
    loadUsersPage();
}

void MainWindow::on_btnReports_clicked()
{
    // Set the stackedWidget index
    ui->stackedWidget->setCurrentIndex(4);
    
    // Uncheck other buttons
    ui->btnMaterials->setChecked(false);
    ui->btnDashboard->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnSuppliers"))
        ui->sidebarWidget->findChild<QPushButton*>("btnSuppliers")->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnUsers"))
        ui->sidebarWidget->findChild<QPushButton*>("btnUsers")->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnSettings"))
        ui->sidebarWidget->findChild<QPushButton*>("btnSettings")->setChecked(false);
    
    // Set this button as checked
    QPushButton* btnReports = ui->sidebarWidget->findChild<QPushButton*>("btnReports");
    if (btnReports) {
        btnReports->setChecked(true);
    }
    
    // Load reports page (placeholder)
    loadReportsPage();
}

void MainWindow::on_btnSettings_clicked()
{
    // Set the stackedWidget index
    ui->stackedWidget->setCurrentIndex(5);
    
    // Uncheck other buttons
    ui->btnMaterials->setChecked(false);
    ui->btnDashboard->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnSuppliers"))
        ui->sidebarWidget->findChild<QPushButton*>("btnSuppliers")->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnUsers"))
        ui->sidebarWidget->findChild<QPushButton*>("btnUsers")->setChecked(false);
    if (ui->sidebarWidget->findChild<QPushButton*>("btnReports"))
        ui->sidebarWidget->findChild<QPushButton*>("btnReports")->setChecked(false);
    
    // Set this button as checked
    QPushButton* btnSettings = ui->sidebarWidget->findChild<QPushButton*>("btnSettings");
    if (btnSettings) {
        btnSettings->setChecked(true);
    }
    
    // Load settings page (placeholder)
    loadSettingsPage();
}

void MainWindow::loadSuppliersPage()
{
    // This would be implemented to create and show the suppliers page
    QMessageBox::information(this, tr("Suppliers"), tr("Suppliers management page will be implemented in a future update."));
}

void MainWindow::loadUsersPage()
{
    // This would be implemented to create and show the users page
    QMessageBox::information(this, tr("Users"), tr("Users management page will be implemented in a future update."));
}

void MainWindow::loadReportsPage()
{
    // This would be implemented to create and show the reports page
    QMessageBox::information(this, tr("Reports"), tr("Reports generation page will be implemented in a future update."));
}

void MainWindow::loadSettingsPage()
{
    // This would be implemented to create and show the settings page
    QMessageBox::information(this, tr("Settings"), tr("Settings page will be implemented in a future update."));
}

// Date filter handlers
void MainWindow::on_dateEditFrom_dateChanged(const QDate &date)
{
    // Ensure from date doesn't exceed to date
    if (date > m_dateEditTo->date()) {
        m_dateEditFrom->setDate(m_dateEditTo->date());
        return;
    }
    
    // Apply filters
    applyFilters();
}

void MainWindow::on_dateEditTo_dateChanged(const QDate &date)
{
    // Ensure to date isn't before from date
    if (date < m_dateEditFrom->date()) {
        m_dateEditTo->setDate(m_dateEditFrom->date());
        return;
    }
    
    // Apply filters
    applyFilters();
}

// Low stock filter handler
void MainWindow::on_checkBoxLowStock_toggled(bool checked)
{
    // Apply filters with low stock flag
    applyFilters();
}

// Database action slots
void MainWindow::on_actionBackupDatabase_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Backup Database"),
                                                 "", tr("SQLite Database (*.db)"));
    if (fileName.isEmpty()) {
        return;
    }
    
    // Add .db extension if not present
    if (!fileName.endsWith(".db", Qt::CaseInsensitive)) {
        fileName += ".db";
    }
    
    if (backupDatabase(fileName)) {
        QMessageBox::information(this, tr("Backup Complete"),
                               tr("Database backup completed successfully."));
    } else {
        QMessageBox::warning(this, tr("Backup Error"),
                            tr("Failed to backup database."));
    }
}

void MainWindow::on_actionRestoreDatabase_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Restore Database"),
                                                 "", tr("SQLite Database (*.db)"));
    if (fileName.isEmpty()) {
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Confirm Restore"),
        tr("Restoring the database will overwrite all current data. Are you sure you want to continue?"),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (restoreDatabase(fileName)) {
            QMessageBox::information(this, tr("Restore Complete"),
                                   tr("Database restore completed successfully. The application will now restart."));
            
            // Restart application
            qApp->quit();
            QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
        } else {
            QMessageBox::warning(this, tr("Restore Error"),
                                tr("Failed to restore database."));
        }
    }
}

void MainWindow::on_actionSettings_triggered()
{
    // Show settings dialog
    loadSettingsPage();
}

// Data management methods
bool MainWindow::backupDatabase(const QString &filePath)
{
    // Close the database connection
    if (m_db.isOpen()) {
        m_db.close();
    }
    
    // Copy the database file
    bool success = QFile::copy("materials.db", filePath);
    
    // Reopen the database
    m_db.open();
    
    return success;
}

bool MainWindow::restoreDatabase(const QString &filePath)
{
    // Close the database connection
    if (m_db.isOpen()) {
        m_db.close();
    }
    
    // Backup current database
    QString backupPath = "materials.db.bak";
    QFile::copy("materials.db", backupPath);
    
    // Replace with new database
    QFile currentDb("materials.db");
    if (currentDb.exists()) {
        currentDb.remove();
    }
    
    bool success = QFile::copy(filePath, "materials.db");
    
    // If failed, restore from backup
    if (!success) {
        QFile::copy(backupPath, "materials.db");
    }
    
    // Reopen the database
    m_db.open();
    
    return success;
}

// Chart methods are now implemented directly in updateCharts()

// Barcode/QR scanning methods
void MainWindow::setupBarcodeScanner()
{
    // Create a button for barcode scanning
    QToolButton *scanButton = new QToolButton(this);
    scanButton->setText(tr("Scan"));
    scanButton->setIcon(QIcon::fromTheme("camera", QIcon(":/icons/barcode.svg"))); // Use system icon if available
    scanButton->setToolTip(tr("Scan Barcode/QR Code"));
    scanButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    // Find the existing file operations toolbar or create a new one
    QToolBar *toolBar = findChild<QToolBar*>("fileOperationsToolbar");
    if (toolBar) {
        // Add to the existing toolbar
        toolBar->addSeparator();
        toolBar->addWidget(scanButton);
    } else {
        // Create a new toolbar for scanning
        QToolBar *scanToolbar = new QToolBar(tr("Scanning"), this);
        scanToolbar->setObjectName("scanningToolbar");
        scanToolbar->setMovable(false);
        scanToolbar->setIconSize(QSize(24, 24));
        scanToolbar->addWidget(scanButton);
        
        // Add toolbar to main window
        addToolBar(Qt::TopToolBarArea, scanToolbar);
        
        // Apply the same style as other toolbars
        QString toolbarStyle = "QToolBar { background-color: #3D485A; border-bottom: 1px solid #2A3340; spacing: 5px; } "
                              "QToolButton { background-color: #E3C6B0; color: #2A3340; border: none; padding: 5px 10px; border-radius: 4px; } "
                              "QToolButton:hover { background-color: #D4B7A1; } "
                              "QToolButton::menu-indicator { image: none; }";
        scanToolbar->setStyleSheet(toolbarStyle);
    }
    
    // Connect the scan button to the barcode scanning function
    connect(scanButton, &QToolButton::clicked, this, [this]() {
        // In a real application, this would open a camera view or scanner
        // For demonstration, we'll simulate with a dialog
        QString barcode = QInputDialog::getText(this, tr("Scan Barcode/QR"),
                                             tr("Enter barcode or scan with device:"));
        if (!barcode.isEmpty()) {
            processBarcodeResult(barcode);
        }
    });
    
    // Add a keyboard shortcut for scanning (Ctrl+B)
    QAction *scanAction = new QAction(tr("Scan Barcode"), this);
    scanAction->setShortcut(QKeySequence("Ctrl+B"));
    connect(scanAction, &QAction::triggered, scanButton, &QToolButton::click);
    addAction(scanAction);
    
    // Also add to the menu
    QMenu *toolsMenu = menuBar()->findChild<QMenu*>("menuTools");
    if (!toolsMenu) {
        toolsMenu = menuBar()->addMenu(tr("&Tools"));
        toolsMenu->setObjectName("menuTools");
    }
    toolsMenu->addAction(scanAction);
}

void MainWindow::processBarcodeResult(const QString &barcode)
{
    // Check if the barcode corresponds to a material ID or SKU
    bool found = false;
    
    // Search for the barcode in the material model
    for (int i = 0; i < m_materialModel->rowCount(); ++i) {
        // Assuming column 1 contains the material name or another column contains SKU/barcode
        // You might need to adjust this based on your data model
        QString materialId = m_materialModel->data(m_materialModel->index(i, 0)).toString();
        QString materialName = m_materialModel->data(m_materialModel->index(i, 1)).toString();
        
        // Check if barcode matches ID or name (in a real app, you'd have a dedicated barcode field)
        if (materialId == barcode || materialName.contains(barcode, Qt::CaseInsensitive)) {
            // Select this row in the table view
            ui->tableViewMaterials->selectRow(i);
            
            // Load the material details
            loadMaterialDetails(materialId.toInt());
            
            // Switch to materials tab if not already there
            on_btnMaterials_clicked();
            
            found = true;
            QMessageBox::information(this, tr("Material Found"),
                                   tr("Found material: %1").arg(materialName));
            break;
        }
    }
    
    if (!found) {
        QMessageBox::information(this, tr("Barcode Result"),
                               tr("No matching material found for barcode: %1\nWould you like to add a new material with this code?").arg(barcode));
        
        // Clear the form for a new entry
        clearForm();
        
        // Pre-fill with barcode information
        // In a real application, you might want to query an external database
        // or API to get product information based on the barcode
        ui->lineEditName->setText(tr("Item: %1").arg(barcode));
    }
}

// Check for low stock items and show notifications
void MainWindow::checkLowStockItems(int threshold)
{
    // If threshold is -1, use the value from the spinbox
    if (threshold == -1) {
        QSpinBox* thresholdSpinBox = findChild<QSpinBox*>("lowStockThresholdSpinBox");
        if (thresholdSpinBox) {
            threshold = thresholdSpinBox->value();
        } else {
            threshold = 10; // Default threshold if spinbox not found
        }
    }
    
    // Get list of low stock items
    QStringList lowStockItems;
    
    for (int i = 0; i < m_materialModel->rowCount(); ++i) {
        int quantity = m_materialModel->data(m_materialModel->index(i, 4)).toInt();
        QString name = m_materialModel->data(m_materialModel->index(i, 1)).toString();
        
        if (quantity <= threshold) {
            lowStockItems.append(QString("%1 (Qty: %2)").arg(name).arg(quantity));
        }
    }
    
    // Show notification if there are low stock items
    if (!lowStockItems.isEmpty()) {
        // Create a notification widget in the status bar
        QLabel* notificationLabel = new QLabel(this);
        notificationLabel->setStyleSheet("background-color: #FFA726; color: white; padding: 5px; border-radius: 3px;");
        notificationLabel->setText(tr("⚠️ Low Stock Alert: %1 items below threshold").arg(lowStockItems.size()));
        
        // Add tooltip with list of low stock items
        QString tooltipText = tr("Low Stock Items:\n") + lowStockItems.join("\n");
        notificationLabel->setToolTip(tooltipText);
        
        // Add to status bar for 10 seconds
        ui->statusbar->addWidget(notificationLabel);
        
        // Create a timer to remove the notification after 10 seconds
        QTimer* timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, [this, notificationLabel]() {
            ui->statusbar->removeWidget(notificationLabel);
            notificationLabel->deleteLater();
        });
        timer->start(10000); // 10 seconds
        
        // Also update the status label with a warning
        if (m_statusLabel) {
            m_statusLabel->setText(tr("Warning: %1 items are low in stock").arg(lowStockItems.size()));
            m_statusLabel->setStyleSheet("color: #FFA726; font-weight: bold;"); // Warning color
        }
    }
}

// This function was a duplicate of on_pushButtonSave_clicked() defined at line 233
// Removing the duplicate implementation to fix the redefinition error
/*
void MainWindow::on_pushButtonSave_clicked()
{
    // Duplicate function removed
*/
