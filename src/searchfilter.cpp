#include "searchfilter.h"
#include "ui_searchfilter.h"
#include <QIcon>

SearchFilter::SearchFilter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchFilter),
    m_sortAscending(true)
{
    ui->setupUi(this);
    setupCategories();
    setupSortCriteria();
    updateSortButtonIcon();
    
    // Initialiser les dates
    ui->checkBoxDateRange->setChecked(false);
    ui->dateEditDebut->setEnabled(false);
    ui->dateEditFin->setEnabled(false);
    ui->dateEditDebut->setDate(QDate::currentDate().addMonths(-1));
    ui->dateEditFin->setDate(QDate::currentDate().addMonths(1));
    
    // Connexions
    connect(ui->checkBoxDateRange, &QCheckBox::toggled, ui->dateEditDebut, &QDateEdit::setEnabled);
    connect(ui->checkBoxDateRange, &QCheckBox::toggled, ui->dateEditFin, &QDateEdit::setEnabled);
}

SearchFilter::~SearchFilter()
{
    delete ui;
}

QString SearchFilter::getSearchTerm() const
{
    return ui->lineEditSearch->text();
}

QString SearchFilter::getCategorie() const
{
    if (ui->comboBoxCategorie->currentIndex() == 0) {
        return QString(); // Toutes les catégories
    }
    return ui->comboBoxCategorie->currentText();
}

QDate SearchFilter::getDateDebut() const
{
    if (ui->checkBoxDateRange->isChecked()) {
        return ui->dateEditDebut->date();
    }
    return QDate(); // Date invalide
}

QDate SearchFilter::getDateFin() const
{
    if (ui->checkBoxDateRange->isChecked()) {
        return ui->dateEditFin->date();
    }
    return QDate(); // Date invalide
}

QString SearchFilter::getSortCriteria() const
{
    switch (ui->comboBoxSort->currentIndex()) {
        case 0: return "nom";
        case 1: return "categorie";
        case 2: return "dateDebut";
        case 3: return "dateFin";
        case 4: return "progression";
        default: return "nom";
    }
}

bool SearchFilter::isSortAscending() const
{
    return m_sortAscending;
}

void SearchFilter::on_btnSearch_clicked()
{
    emit searchRequested();
}

void SearchFilter::on_btnReset_clicked()
{
    ui->lineEditSearch->clear();
    ui->comboBoxCategorie->setCurrentIndex(0);
    ui->checkBoxDateRange->setChecked(false);
    ui->dateEditDebut->setDate(QDate::currentDate().addMonths(-1));
    ui->dateEditFin->setDate(QDate::currentDate().addMonths(1));
    ui->comboBoxSort->setCurrentIndex(0);
    m_sortAscending = true;
    updateSortButtonIcon();
    
    emit resetRequested();
}

void SearchFilter::on_comboBoxSort_currentIndexChanged(int index)
{
    emit sortRequested(getSortCriteria(), m_sortAscending);
}

void SearchFilter::on_btnSortDirection_clicked()
{
    m_sortAscending = !m_sortAscending;
    updateSortButtonIcon();
    emit sortRequested(getSortCriteria(), m_sortAscending);
}

void SearchFilter::setupCategories()
{
    ui->comboBoxCategorie->clear();
    ui->comboBoxCategorie->addItem(tr("Toutes les catégories"));
    ui->comboBoxCategorie->addItem(tr("Résidentiel"));
    ui->comboBoxCategorie->addItem(tr("Commercial"));
    ui->comboBoxCategorie->addItem(tr("Industriel"));
    ui->comboBoxCategorie->addItem(tr("Infrastructure"));
    ui->comboBoxCategorie->addItem(tr("Autre"));
}

void SearchFilter::setupSortCriteria()
{
    ui->comboBoxSort->clear();
    ui->comboBoxSort->addItem(tr("Nom"));
    ui->comboBoxSort->addItem(tr("Catégorie"));
    ui->comboBoxSort->addItem(tr("Date de début"));
    ui->comboBoxSort->addItem(tr("Date de fin"));
    ui->comboBoxSort->addItem(tr("Progression"));
}

void SearchFilter::updateSortButtonIcon()
{
    if (m_sortAscending) {
        ui->btnSortDirection->setText(tr("↑"));
        ui->btnSortDirection->setToolTip(tr("Tri ascendant"));
    } else {
        ui->btnSortDirection->setText(tr("↓"));
        ui->btnSortDirection->setToolTip(tr("Tri descendant"));
    }
}
