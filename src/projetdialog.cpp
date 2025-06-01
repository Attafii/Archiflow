#include "projetdialog.h"
#include "ui_projetdialog.h"
#include "durationestimator.h"
#include <QMessageBox>

ProjetDialog::ProjetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjetDialog),
    m_editMode(false),
    m_projetId(0)
{
    ui->setupUi(this);
    setupCategories();
    setupConnections();
    
    // Initialiser les dates
    ui->dateEditDebut->setDate(QDate::currentDate());
    ui->dateEditFin->setDate(QDate::currentDate().addDays(30));
    
    // Mode création par défaut
    setWindowTitle(tr("Nouveau Projet"));
}

ProjetDialog::~ProjetDialog()
{
    delete ui;
}

void ProjetDialog::setEditMode(bool editMode)
{
    m_editMode = editMode;
    if (editMode) {
        setWindowTitle(tr("Modifier Projet"));
    } else {
        setWindowTitle(tr("Nouveau Projet"));
    }
}

void ProjetDialog::setProjet(const Projet &projet)
{
    m_projetId = projet.getId();
    ui->lineEditNom->setText(projet.getNom());
    ui->comboBoxCategorie->setCurrentText(projet.getCategorie());
    ui->dateEditDebut->setDate(projet.getDateDebut());
    ui->dateEditFin->setDate(projet.getDateFinEstimee());
    ui->spinBoxProgression->setValue(projet.getProgression());
}

Projet ProjetDialog::getProjet() const
{
    Projet projet(
        m_projetId,
        ui->lineEditNom->text(),
        ui->comboBoxCategorie->currentText(),
        ui->dateEditDebut->date(),
        ui->dateEditFin->date()
    );
    
    projet.setProgression(ui->spinBoxProgression->value());
    
    return projet;
}

void ProjetDialog::on_buttonBox_accepted()
{
    // Validation
    if (ui->lineEditNom->text().isEmpty()) {
        QMessageBox::warning(this, tr("Validation"), tr("Le nom du projet est obligatoire."));
        return;
    }
    
    if (ui->dateEditDebut->date() > ui->dateEditFin->date()) {
        QMessageBox::warning(this, tr("Validation"), 
                            tr("La date de début doit être antérieure à la date de fin."));
        return;
    }
    
    accept();
}

void ProjetDialog::on_buttonBox_rejected()
{
    reject();
}

void ProjetDialog::on_comboBoxCategorie_currentIndexChanged(const QString &categorie)
{
    updateDureeEstimee();
}

void ProjetDialog::updateDureeEstimee()
{
    // Utiliser l'estimateur pour mettre à jour la date de fin estimée
    QString categorie = ui->comboBoxCategorie->currentText();
    int complexite = ui->spinBoxComplexite->value();
    
    DurationEstimator estimator;
    int dureeEstimee = estimator.estimerDuree(categorie, complexite);
    
    QDate dateDebut = ui->dateEditDebut->date();
    QDate dateFin = dateDebut.addDays(dureeEstimee);
    
    // Mettre à jour la date de fin sans déclencher de signaux
    ui->dateEditFin->blockSignals(true);
    ui->dateEditFin->setDate(dateFin);
    ui->dateEditFin->blockSignals(false);
    
    // Afficher la durée estimée
    ui->labelDureeEstimee->setText(tr("%1 jours").arg(dureeEstimee));
}

void ProjetDialog::setupCategories()
{
    ui->comboBoxCategorie->clear();
    ui->comboBoxCategorie->addItem(tr("Résidentiel"));
    ui->comboBoxCategorie->addItem(tr("Commercial"));
    ui->comboBoxCategorie->addItem(tr("Industriel"));
    ui->comboBoxCategorie->addItem(tr("Infrastructure"));
    ui->comboBoxCategorie->addItem(tr("Autre"));
}

void ProjetDialog::setupConnections()
{
    connect(ui->spinBoxComplexite, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &ProjetDialog::updateDureeEstimee);
    
    connect(ui->dateEditDebut, &QDateEdit::dateChanged, 
            this, &ProjetDialog::updateDureeEstimee);
}
