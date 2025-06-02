# Database Schema for Qt Invoice Application (SQLite)

This document outlines the proposed database schema for the "Facture Architecte Intelligent" application.

## 1. Clients Table

Stores information about the clients.

- **Table Name:** `Clients`
- **Columns:**
    - `client_id` INTEGER PRIMARY KEY AUTOINCREMENT: Unique identifier for the client.
    - `name` TEXT NOT NULL: Name of the client.
    - `address` TEXT: Client's address.
    - `email` TEXT: Client's email address.
    - `phone` TEXT: Client's phone number.

**SQL:**
```sql
CREATE TABLE Clients (
    client_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    address TEXT,
    email TEXT,
    phone TEXT
);
```

## 2. Invoices Table

Stores the main details for each invoice.

- **Table Name:** `Invoices`
- **Columns:**
    - `invoice_id` INTEGER PRIMARY KEY AUTOINCREMENT: Unique identifier for the invoice (used for "Numéro de facture").
    - `client_id` INTEGER: Foreign key referencing `Clients.client_id`.
    - `invoice_date` TEXT: Date the invoice was created (e.g., 'YYYY-MM-DD').
    - `payment_mode` TEXT: Method of payment (e.g., 'Espèces', 'Virement', 'Carte').
    - `status` TEXT: Current status of the invoice (e.g., 'En attente', 'Payée', 'Annulée').
    - `sub_total` REAL: The total amount before VAT.
    - `vat_rate` REAL: The VAT rate applied (e.g., 0.20 for 20%).
    - `vat_amount` REAL: The calculated VAT amount.
    - `total_amount` REAL: The final total amount (sub_total + vat_amount).

**SQL:**
```sql
CREATE TABLE Invoices (
    invoice_id INTEGER PRIMARY KEY AUTOINCREMENT,
    client_id INTEGER,
    invoice_date TEXT,
    payment_mode TEXT,
    status TEXT,
    sub_total REAL,
    vat_rate REAL,
    vat_amount REAL,
    total_amount REAL,
    FOREIGN KEY (client_id) REFERENCES Clients (client_id)
);
```

## 3. InvoiceItems Table

Stores individual items associated with an invoice.

- **Table Name:** `InvoiceItems`
- **Columns:**
    - `item_id` INTEGER PRIMARY KEY AUTOINCREMENT: Unique identifier for the invoice item line.
    - `invoice_id` INTEGER: Foreign key referencing `Invoices.invoice_id`.
    - `description` TEXT: Description of the item or service.
    - `quantity` REAL: Quantity of the item.
    - `unit_price` REAL: Price per unit of the item.
    - `total_price` REAL: Calculated total for this line item (quantity * unit_price).

**SQL:**
```sql
CREATE TABLE InvoiceItems (
    item_id INTEGER PRIMARY KEY AUTOINCREMENT,
    invoice_id INTEGER,
    description TEXT,
    quantity REAL,
    unit_price REAL,
    total_price REAL,
    FOREIGN KEY (invoice_id) REFERENCES Invoices (invoice_id)
);
```

