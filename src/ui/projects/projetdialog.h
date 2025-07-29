#ifndef PROJETDIALOG_H
#define PROJETDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QValidator>
#include <QRegularExpressionValidator>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QCompleter>
#include <QStandardItemModel>
#include <QDateTime>
#include <QDebug>

#include "../../features/projects/projet.h"
#include "../../features/projects/projetmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ProjetDialog; }
QT_END_NAMESPACE

/**
 * @brief The ProjetDialog class - UI Controller for project CRUD operations
 * 
 * This dialog provides a comprehensive interface for creating, editing, and viewing
 * architectural projects. It handles form validation, data binding, and user interactions.
 */
class ProjetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjetDialog(ProjetManager *projetManager, QWidget *parent = nullptr);
    explicit ProjetDialog(ProjetManager *projetManager, const Projet &projet, QWidget *parent = nullptr);
    ~ProjetDialog();

    // Static convenience methods
    static bool createNewProject(ProjetManager *projetManager, QWidget *parent = nullptr);
    static bool editProject(ProjetManager *projetManager, const Projet &projet, QWidget *parent = nullptr);
    static bool viewProject(ProjetManager *projetManager, const Projet &projet, QWidget *parent = nullptr);

    // Getters
    Projet getProjet() const { return m_currentProjet; }
    bool isModified() const { return m_isModified; }

public slots:
    void accept() override;
    void reject() override;

private slots:
    // Form validation slots
    void validateForm();
    void onFormDataChanged();
    
    // UI event handlers
    void on_lineEditNom_textChanged(const QString &text);
    void on_textEditDescription_textChanged();
    void on_comboBoxCategorie_currentTextChanged(const QString &text);
    void on_comboBoxStatut_currentTextChanged(const QString &text);
    void on_lineEditClient_textChanged(const QString &text);
    void on_lineEditArchitecte_textChanged(const QString &text);
    void on_doubleSpinBoxBudget_valueChanged(double value);
    void on_doubleSpinBoxSurface_valueChanged(double value);
    void on_spinBoxEtage_valueChanged(int value);
    void on_comboBoxMateriauPrincipal_currentTextChanged(const QString &text);    void on_dateEditDebut_dateChanged(const QDate &date);
    void on_dateEditFinEstimee_dateChanged(const QDate &date);
    void on_sliderProgression_valueChanged(int value);
    void on_spinBoxProgression_valueChanged(int value);
      // Location handling
    void on_doubleSpinBoxLatitude_valueChanged(double value);
    void on_doubleSpinBoxLongitude_valueChanged(double value);
    void on_lineEditAdresse_textChanged(const QString &text);
    void on_pushButtonRechercherLocalisation_clicked();
    
    // Action buttons
    void on_pushButtonSauvegarder_clicked();
    void on_pushButtonAnnuler_clicked();
    void on_pushButtonSupprimer_clicked();
    void on_pushButtonReinitialiser_clicked();

signals:
    void projetSaved(const Projet &projet);
    void projetDeleted(int projetId);
    void projetModified(const Projet &projet);

private:
    // Core methods
    void setupUI();
    void setupValidators();
    void setupConnections();
    void setupCompleters();
    void populateComboBoxes();
    void loadProjectData();
    void saveProjectData();
    void resetForm();
    void setFormReadOnly(bool readOnly);
    
    // Validation methods
    bool validateRequiredFields();
    bool validateCoordinates();
    bool validateDates();
    bool validateBudget();
    bool validateProgression();
    QString getValidationErrors();
    
    // Helper methods
    void updateProgressionDisplay();
    void updateDurationEstimate();
    void showValidationMessage(const QString &message);
    void setFieldError(QWidget *widget, bool hasError);
    void clearFieldErrors();
    
    // Data conversion methods
    void projectToForm(const Projet &projet);
    Projet formToProject();
    
private:
    Ui::ProjetDialog *ui;
    ProjetManager *m_projetManager;
    Projet m_currentProjet;
    Projet m_originalProjet; // For comparison to detect changes
    
    // State flags
    bool m_isEditMode;
    bool m_isViewMode;
    bool m_isModified;
    bool m_isNewProject;
    
    // Validators
    QRegularExpressionValidator *m_nameValidator;
    QDoubleValidator *m_coordinateValidator;
    QDoubleValidator *m_budgetValidator;
    QIntValidator *m_progressionValidator;
    
    // Completers for auto-completion
    QCompleter *m_clientCompleter;
    QCompleter *m_architecteCompleter;
    QCompleter *m_adresseCompleter;
    
    // Models for completers
    QStandardItemModel *m_clientModel;
    QStandardItemModel *m_architecteModel;
    
    // UI styling
    void applyArchiFlowStyling();
    void updateButtonStates();
};

#endif // PROJETDIALOG_H
