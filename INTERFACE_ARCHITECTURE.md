# ArchiFlow Interface Architecture

## Overview

The ArchiFlow application uses a comprehensive interface-based architecture to ensure modularity, testability, and maintainability. This document describes the interface patterns and their implementations.

## Interface Hierarchy

### Core Interfaces

#### IDatabaseManager
- **Purpose**: Abstracts database operations for different implementations
- **Location**: `src/interfaces/idatabasemanager.h`
- **Implementations**: `DatabaseManager`
- **Features**:
  - Connection management
  - Query execution
  - Transaction support
  - Schema management
  - Signal-based error handling

#### IModuleInterface
- **Purpose**: Defines the contract for all feature modules
- **Location**: `src/interfaces/imoduleinterface.h`
- **Implementations**: `BaseModule` and all feature modules
- **Features**:
  - Module lifecycle management
  - UI integration
  - Dependency resolution
  - Configuration management

### Contract Feature Interfaces

#### IContractService
- **Purpose**: Defines business logic and data operations for contracts
- **Location**: `src/interfaces/icontractservice.h`
- **Implementations**: `ContractDatabaseManager`
- **Features**:
  - CRUD operations
  - Search and filtering
  - Statistics and analytics
  - Validation
  - Export/Import

#### IContractWidget
- **Purpose**: Defines the contract for contract UI widgets
- **Location**: `src/interfaces/icontractwidget.h`
- **Implementations**: `ContractWidget`
- **Features**:
  - Data display and management
  - User interactions
  - Search and filtering
  - View modes and configuration
  - Export functionality

#### IContractDialog
- **Purpose**: Defines the contract for contract dialog implementations
- **Location**: `src/interfaces/icontractdialog.h`
- **Implementations**: `ContractDialog`
- **Features**:
  - Multiple dialog modes (Create, Edit, View, Duplicate)
  - Form validation
  - Data binding
  - State management

## Benefits

### 1. **Modularity**
- Clear separation of concerns
- Easy to add new implementations
- Reduced coupling between components

### 2. **Testability**
- Easy to mock interfaces for unit testing
- Test different implementations independently
- Clear test boundaries

### 3. **Maintainability**
- Changes to implementation don't affect interface consumers
- Clear contracts reduce bugs
- Easy to understand code structure

### 4. **Extensibility**
- New features can implement existing interfaces
- Easy to add new interface methods
- Supports multiple implementations

## Usage Patterns

### Service Injection
```cpp
// Widget receives service through interface
ContractWidget *widget = new ContractWidget();
widget->setContractService(contractService); // IContractService*
```

### Interface Implementation
```cpp
class ContractDatabaseManager : public QObject, public IContractService
{
    Q_OBJECT
public:
    // Implement all IContractService methods
    QString addContract(Contract *contract) override;
    bool updateContract(Contract *contract) override;
    // ... other methods
};
```

### Signal-Based Interfaces
```cpp
class IContractWidgetSignals : public QWidget, public IContractWidget
{
    Q_OBJECT
signals:
    void contractAdded(const QString &contractId);
    void contractUpdated(const QString &contractId);
    // ... other signals
};
```

## Best Practices

### 1. **Interface Design**
- Keep interfaces focused and cohesive
- Use pure virtual functions for core functionality
- Provide default implementations where appropriate
- Use Qt's signal-slot mechanism for event-driven behavior

### 2. **Implementation**
- Always implement all interface methods
- Use override keyword for clarity
- Handle error cases gracefully
- Emit appropriate signals for state changes

### 3. **Dependency Injection**
- Pass interfaces, not concrete implementations
- Use setter methods for dependency injection
- Check for null pointers before using injected dependencies

### 4. **Backward Compatibility**
- Keep existing interface methods stable
- Add new methods carefully (consider default implementations)
- Use deprecation warnings before removing methods

## Future Extensions

### Planned Interfaces
- `IMaterialService` - Material management business logic
- `IProjectService` - Project management operations  
- `IEmployeeService` - Employee management functionality
- `IClientService` - Client relationship management
- `IReportService` - Report generation and export
- `IAIService` - AI integration and chatbot functionality

### Interface Patterns to Add
- Repository pattern interfaces for data access
- Factory interfaces for object creation
- Observer pattern interfaces for event handling
- Strategy pattern interfaces for algorithm selection

## Migration Guide

### For Existing Code
1. Update class declarations to implement interfaces
2. Add override keywords to method implementations
3. Update dependency injection to use interfaces
4. Add any missing interface methods

### For New Features
1. Design interface first, then implementation
2. Consider existing patterns and naming conventions
3. Add comprehensive documentation
4. Include interface in main interfaces.h header

## Testing

### Interface Testing
- Create mock implementations for testing
- Test interface contracts, not implementations
- Use Qt Test framework for automated testing
- Test signal-slot connections

### Example Test Structure
```cpp
class MockContractService : public IContractService
{
    // Mock implementation for testing
};

class ContractWidgetTest : public QObject
{
    Q_OBJECT
private slots:
    void testContractAddition();
    void testSearchFiltering();
};
```

This interface architecture provides a solid foundation for the ArchiFlow application's modular design and ensures consistent patterns across all features.
