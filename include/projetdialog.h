#ifndef PROJETDIALOG_H
#define PROJETDIALOG_H

#include <QDialog>
#include <QDate>
#include "projet.h"

namespace Ui {
class ProjetDialog;
}

class ProjetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjetDialog(QWidget *parent = nullptr);
    ~ProjetDialog();
    
    void setEditMode(bool editMode);
    void setProjet(const Projet &projet);
    Projet getProjet() const;
    
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_comboBoxCategorie_currentIndexChanged(const QString &categorie);
    void updateDureeEstimee();

private:
    Ui::ProjetDialog *ui;
    bool m_editMode;
    int m_projetId;
    
    void setupCategories();
    void setupConnections();
};

#endif // PROJETDIALOG_H
