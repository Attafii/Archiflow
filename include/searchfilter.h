#ifndef SEARCHFILTER_H
#define SEARCHFILTER_H

#include <QWidget>
#include <QDate>

namespace Ui {
class SearchFilter;
}

class SearchFilter : public QWidget
{
    Q_OBJECT

public:
    explicit SearchFilter(QWidget *parent = nullptr);
    ~SearchFilter();

    QString getSearchTerm() const;
    QString getCategorie() const;
    QDate getDateDebut() const;
    QDate getDateFin() const;
    QString getSortCriteria() const;
    bool isSortAscending() const;

signals:
    void searchRequested();
    void resetRequested();
    void sortRequested(const QString &criteria, bool ascending);

private slots:
    void on_btnSearch_clicked();
    void on_btnReset_clicked();
    void on_comboBoxSort_currentIndexChanged(int index);
    void on_btnSortDirection_clicked();

private:
    Ui::SearchFilter *ui;
    bool m_sortAscending;

    void setupCategories();
    void setupSortCriteria();
    void updateSortButtonIcon();
};

#endif // SEARCHFILTER_H
