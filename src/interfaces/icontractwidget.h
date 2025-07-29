#ifndef ICONTRACTWIDGET_H
#define ICONTRACTWIDGET_H

#include <QWidget>
#include <QString>
#include <QList>

class Contract;
class IContractService;

/**
 * @brief Interface for contract widget implementations
 * 
 * This interface defines the contract for all contract UI widgets,
 * ensuring consistent user interface patterns across different implementations.
 */
class IContractWidget
{
public:
    virtual ~IContractWidget() = default;

    // Data management
    virtual void setContractService(IContractService *service) = 0;
    virtual void refreshContracts() = 0;
    virtual void clearSelection() = 0;

    // Contract operations
    virtual void addContract() = 0;
    virtual void editContract(const QString &contractId) = 0;
    virtual void deleteContract(const QString &contractId) = 0;
    virtual void duplicateContract(const QString &contractId) = 0;

    // Search and filter
    virtual void setSearchFilter(const QString &filter) = 0;
    virtual void setStatusFilter(const QString &status) = 0;
    virtual void setDateRangeFilter(const QDate &startDate, const QDate &endDate) = 0;
    virtual void clearFilters() = 0;

    // Selection management
    virtual QList<QString> getSelectedContractIds() = 0;
    virtual Contract* getCurrentContract() = 0;
    virtual void selectContract(const QString &contractId) = 0;

    // Export functionality
    virtual void exportSelectedContracts() = 0;
    virtual void exportAllContracts() = 0;

    // View configuration
    virtual void setViewMode(int mode) = 0; // 0: List, 1: Grid, 2: Cards
    virtual void setSortOrder(int column, bool ascending = true) = 0;
    virtual void setColumnsVisible(const QStringList &columns) = 0;

    // Statistics display
    virtual void showStatistics() = 0;
    virtual void showContractDetails(const QString &contractId) = 0;
};

/**
 * @brief Signal definitions for contract widget events
 * 
 * This class provides signal definitions that contract widgets should emit
 * for event-driven UI operations. Widget implementations should declare
 * these signals in their class definitions.
 */
class IContractWidgetSignals
{
public:
    virtual ~IContractWidgetSignals() = default;

    // Virtual signal declarations - concrete classes should implement as Qt signals
    virtual void contractAdded(const QString &contractId) = 0;
    virtual void contractUpdated(const QString &contractId) = 0;
    virtual void contractDeleted(const QString &contractId) = 0;
    virtual void contractSelected(const QString &contractId) = 0;
    virtual void contractDoubleClicked(const QString &contractId) = 0;
    virtual void selectionChanged() = 0;
    virtual void filterChanged() = 0;
    virtual void exportRequested() = 0;
    virtual void statisticsRequested() = 0;
    virtual void errorOccurred(const QString &error) = 0;
};

#endif // ICONTRACTWIDGET_H
