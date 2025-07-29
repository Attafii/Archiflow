#include "migrations.h"
#include "databasemanager.h"
#include <QDebug>
#include <algorithm>

Migrations::Migrations(DatabaseManager *databaseManager, QObject *parent)
    : QObject(parent)
    , m_databaseManager(databaseManager)
{
    registerCoreMigrations();
}

bool Migrations::createInitialSchema()
{
    qDebug() << "Creating initial database schema...";
    
    // The initial schema will be created through migrations
    // This allows for consistent versioning from the start
    return true;
}

bool Migrations::runMigrations()
{
    qDebug() << "Running database migrations...";
    
    int currentVersion = m_databaseManager->currentSchemaVersion();
    qDebug() << "Current schema version:" << currentVersion;
    
    // Sort migrations by version
    std::sort(m_migrations.begin(), m_migrations.end(),
              [](const Migration &a, const Migration &b) {
                  return a.version < b.version;
              });
    
    bool allSuccessful = true;
    for (const Migration &migration : m_migrations) {
        if (migration.version <= currentVersion) {
            continue; // Skip already applied migrations
        }
        
        qDebug() << "Applying migration" << migration.version << ":" << migration.description;
        
        if (!executeMigration(migration)) {
            qCritical() << "Migration" << migration.version << "failed!";
            allSuccessful = false;
            break;
        }
        
        qDebug() << "Migration" << migration.version << "applied successfully";
    }
    
    if (allSuccessful) {
        qDebug() << "All migrations completed successfully";
    }
    
    return allSuccessful;
}

void Migrations::addMigration(int version, const QString &description, std::function<bool()> migration)
{
    m_migrations.append({version, description, migration});
}

bool Migrations::executeMigration(const Migration &migration)
{
    if (!m_databaseManager->beginTransaction()) {
        return false;
    }
    
    bool success = migration.execute();
    
    if (success) {
        // Record the migration
        success = m_databaseManager->executeNonQuery(
            "INSERT INTO schema_version (version) VALUES (?)",
            {migration.version}
        );
    }
    
    if (success) {
        return m_databaseManager->commitTransaction();
    } else {
        m_databaseManager->rollbackTransaction();
        return false;
    }
}

void Migrations::registerCoreMigrations()
{
    // Migration 1: Create core application tables
    addMigration(1, "Create core application tables", [this]() {
        // Settings table for application configuration
        bool success = m_databaseManager->executeNonQuery(R"(
            CREATE TABLE settings (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                key TEXT UNIQUE NOT NULL,
                value TEXT,
                category TEXT DEFAULT 'general',
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        )");
        
        if (!success) return false;
        
        // Audit log table for tracking changes
        success = m_databaseManager->executeNonQuery(R"(
            CREATE TABLE audit_log (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                table_name TEXT NOT NULL,
                record_id INTEGER,
                action TEXT NOT NULL, -- 'INSERT', 'UPDATE', 'DELETE'
                old_values TEXT, -- JSON
                new_values TEXT, -- JSON
                user_id INTEGER,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        )");
        
        if (!success) return false;
        
        // Create indexes
        success = m_databaseManager->executeNonQuery(
            "CREATE INDEX idx_settings_key ON settings(key)"
        );
        if (!success) return false;
        
        success = m_databaseManager->executeNonQuery(
            "CREATE INDEX idx_audit_log_table_record ON audit_log(table_name, record_id)"
        );
        if (!success) return false;
        
        success = m_databaseManager->executeNonQuery(
            "CREATE INDEX idx_audit_log_timestamp ON audit_log(timestamp)"
        );
        
        return success;
    });
    
    // Migration 2: Create users and permissions system
    addMigration(2, "Create users and permissions system", [this]() {
        // Users table
        bool success = m_databaseManager->executeNonQuery(R"(
            CREATE TABLE users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT UNIQUE NOT NULL,
                email TEXT UNIQUE NOT NULL,
                password_hash TEXT NOT NULL,
                full_name TEXT NOT NULL,
                role TEXT NOT NULL DEFAULT 'user', -- 'admin', 'manager', 'user'
                is_active BOOLEAN DEFAULT 1,
                last_login DATETIME,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        )");
        
        if (!success) return false;
        
        // Sessions table for login management
        success = m_databaseManager->executeNonQuery(R"(
            CREATE TABLE user_sessions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER NOT NULL,
                session_token TEXT UNIQUE NOT NULL,
                expires_at DATETIME NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            )
        )");
        
        if (!success) return false;
        
        // Create indexes
        success = m_databaseManager->executeNonQuery(
            "CREATE INDEX idx_users_username ON users(username)"
        );
        if (!success) return false;
        
        success = m_databaseManager->executeNonQuery(
            "CREATE INDEX idx_users_email ON users(email)"
        );
        if (!success) return false;
        
        success = m_databaseManager->executeNonQuery(
            "CREATE INDEX idx_sessions_token ON user_sessions(session_token)"
        );
        if (!success) return false;
        
        // Insert default admin user (password: admin123 - should be changed!)
        success = m_databaseManager->executeNonQuery(R"(
            INSERT INTO users (username, email, password_hash, full_name, role) 
            VALUES ('admin', 'admin@archiflow.local', 
                   '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewrvL.UdW9k0ZQHW', 
                   'System Administrator', 'admin')
        )");
        
        return success;
    });
      // Migration 3: Create materials management tables
    addMigration(3, "Create materials management tables", [this]() {
        // Materials table
        bool success = m_databaseManager->executeNonQuery(R"(
            CREATE TABLE materials (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                description TEXT,
                category TEXT NOT NULL,
                quantity INTEGER NOT NULL DEFAULT 0,
                unit TEXT NOT NULL DEFAULT 'pieces',
                price REAL NOT NULL DEFAULT 0.0,
                supplier_id INTEGER,
                barcode TEXT UNIQUE,
                location TEXT,
                minimum_stock INTEGER DEFAULT 0,
                maximum_stock INTEGER DEFAULT 1000,
                reorder_point INTEGER DEFAULT 10,
                status TEXT DEFAULT 'active' CHECK (status IN ('active', 'inactive', 'discontinued')),
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                created_by TEXT,
                updated_by TEXT
            )
        )");
        
        if (!success) return false;
        
        // Suppliers table
        success = m_databaseManager->executeNonQuery(R"(
            CREATE TABLE suppliers (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL UNIQUE,
                contact_person TEXT,
                email TEXT,
                phone TEXT,
                address TEXT,
                city TEXT,
                country TEXT,
                postal_code TEXT,
                website TEXT,
                notes TEXT,
                rating INTEGER DEFAULT 0 CHECK (rating >= 0 AND rating <= 5),
                status TEXT DEFAULT 'active' CHECK (status IN ('active', 'inactive')),
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        )");
        
        if (!success) return false;
        
        // Material movements table for tracking inventory changes
        success = m_databaseManager->executeNonQuery(R"(
            CREATE TABLE material_movements (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                material_id INTEGER NOT NULL,
                movement_type TEXT NOT NULL CHECK (movement_type IN ('in', 'out', 'adjustment')),
                quantity INTEGER NOT NULL,
                reference TEXT,
                notes TEXT,
                performed_by TEXT,
                movement_date DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (material_id) REFERENCES materials(id) ON DELETE CASCADE
            )
        )");
        
        if (!success) return false;
        
        // Create indexes for better performance
        success = m_databaseManager->executeNonQuery(
            "CREATE INDEX idx_materials_name ON materials(name)"
        );
        if (!success) return false;
        
        success = m_databaseManager->executeNonQuery(
            "CREATE INDEX idx_materials_category ON materials(category)"
        );
        if (!success) return false;
        
        success = m_databaseManager->executeNonQuery(
            "CREATE INDEX idx_materials_status ON materials(status)"
        );
        if (!success) return false;
        
        success = m_databaseManager->executeNonQuery(
            "CREATE INDEX idx_materials_barcode ON materials(barcode)"
        );
        if (!success) return false;
        
        success = m_databaseManager->executeNonQuery(
            "CREATE INDEX idx_suppliers_name ON suppliers(name)"
        );
        if (!success) return false;
        
        success = m_databaseManager->executeNonQuery(
            "CREATE INDEX idx_material_movements_material_id ON material_movements(material_id)"
        );
        if (!success) return false;
        
        success = m_databaseManager->executeNonQuery(
            "CREATE INDEX idx_material_movements_date ON material_movements(movement_date)"
        );
        if (!success) return false;
        
        // Insert some default suppliers
        success = m_databaseManager->executeNonQuery(R"(
            INSERT INTO suppliers (name, contact_person, email, phone, address, city, country, rating) 
            VALUES 
            ('BuildCorp Materials', 'John Smith', 'john@buildcorp.com', '+1-555-0101', '123 Industrial Ave', 'Chicago', 'USA', 5),
            ('SteelWorks Ltd', 'Maria Garcia', 'maria@steelworks.com', '+1-555-0102', '456 Steel Street', 'Pittsburgh', 'USA', 4),
            ('Concrete Solutions', 'David Wilson', 'david@concrete-sol.com', '+1-555-0103', '789 Concrete Blvd', 'Denver', 'USA', 4)
        )");
          return success;
    });

    // Future migrations will be added here as features are implemented
    // Examples:
    // addMigration(5, "Create employees tables", [this]() { ... });
    // addMigration(6, "Create clients tables", [this]() { ... });
    // addMigration(7, "Create projects tables", [this]() { ... });
}
