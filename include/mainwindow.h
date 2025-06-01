#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include "projet.h"
#include "Arduino.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionNouveau_triggered();
    void on_actionModifier_triggered();
    void on_actionSupprimer_triggered();
    void on_actionQuitter_triggered();
    void on_actionAPropos_triggered();
    
    void on_tableViewProjets_doubleClicked(const QModelIndex &index);
    
    void onSearchRequested();
    void onResetRequested();
    void onSortRequested(const QString &criteria, bool ascending);
    
    void updateProjectList();
    void updateStatusBar();

private:
    Ui::MainWindow *ui;
    QList<Projet> m_projets;
    
    void setupConnections();
    void setupTableView();
    void setupTheme();
    void loadProjets();
    int getSelectedProjetId() const;
    void refreshProjetDetails(int projetId);

    // New Arduino-related members and methods
    Arduino *m_arduino;
    QTimer *m_portUpdateTimer;

    void setupArduinoConnections();
    void setupArduinoUI();
    void populatePortList();
    void updateMaterialStock(int stockLevel, int materialId = 1);
    void showStockAlert(const QString &message);
};

#endif // MAINWINDOW_H
