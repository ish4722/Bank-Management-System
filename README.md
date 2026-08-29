# Bank Management System

![Made with C++](https://img.shields.io/badge/Made%20with-C%2B%2B17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge)

A modular **C++17 console-based Bank Management System** for managing savings and current accounts. The project demonstrates object-oriented programming, STL containers, SQLite-backed persistence, ACID transactions, concurrency control, transaction history, CSV export, input validation, and automated testing.

> **Note:** This is an educational/portfolio project. It is not intended to represent a production banking system. It does not provide authentication, authorization, encryption, network security, or the operational controls required for real financial software.

---

## 📌 Features

- ✅ Create Savings and Current accounts
- ✅ Generate unique 10-digit account numbers
- ✅ View account details
- ✅ Modify account name and type
- ✅ Deposit money
- ✅ Withdraw money with balance validation
- ✅ Persistent transaction history
- ✅ View transaction history from the CLI
- ✅ Store transaction type, amount, resulting balance, and timestamp
- ✅ Delete accounts with transaction history removed through foreign-key cascading
- ✅ SQLite database persistence
- ✅ ACID transactions for account mutations
- ✅ Concurrency-safe withdrawals
- ✅ Integer minor-unit storage for monetary values in SQLite
- ✅ WAL mode and SQLite busy timeout for improved concurrent access
- ✅ Export current account state to CSV
- ✅ O(1) average in-memory account lookup using `std::unordered_map`
- ✅ Centralized account management through a `Bank` class
- ✅ Input validation for menu choices, account types, names, and amounts
- ✅ Catch2-based unit/integration tests
- ✅ C++17 and Makefile-based build

---

## 🏗️ Architecture

```text
                         ┌─────────────────────┐
                         │      main.cpp       │
                         │   CLI / User Input  │
                         └──────────┬──────────┘
                                    │
                                    ▼
                         ┌─────────────────────┐
                         │        Bank         │
                         │ Account Management  │
                         └──────────┬──────────┘
                                    │
                   ┌────────────────┴────────────────┐
                   ▼                                 ▼
          unordered_map                         SQLite DB
       <AccountNo, User>                           │
                   │                         ┌───────┴────────┐
                   ▼                         ▼                ▼
                 User                    accounts       transactions
                   │                         │                │
                   ▼                         └───────┬────────┘
             Transactions                            │
                                                     ▼
                                           ACID transaction boundary
                                                     │
                                           ┌─────────┴─────────┐
                                           ▼                   ▼
                                      COMMIT              ROLLBACK
```

### Write path

Every mutating bank operation is persisted through SQLite rather than rewriting a complete JSON document:

```text
Deposit / Withdrawal / Modify / Delete / Create
                     │
                     ▼
          BEGIN IMMEDIATE TRANSACTION
                     │
                     ▼
             Execute SQL statements
                     │
                     ├──────── failure ────────► ROLLBACK
                     │
                     ▼
                   COMMIT
                     │
                     ▼
              Reload in-memory state
```

For deposits and withdrawals, the balance update and transaction-history insert occur inside the **same database transaction**. Therefore either both changes become durable or neither does.

### Design goals

- **Single source of truth:** SQLite is the durable source of account state; `Bank` maintains an in-memory representation for fast CLI access.
- **Separation of concerns:** `User` models account state and transactions, `Bank` manages account operations, and `Database` owns SQLite connection/execution details.
- **Efficient lookup:** account numbers remain keys in an `unordered_map`, providing average O(1) lookup in the application layer.
- **Persistent audit trail:** successful deposits and withdrawals append transaction records to the database.
- **Atomic mutations:** balance changes and their corresponding transaction records are committed together.
- **Concurrency safety:** SQLite write transactions serialize competing writers, while the withdrawal SQL predicate enforces the sufficient-balance invariant at the database level.

---

## 🛠️ Technologies & Concepts

| Technology / Concept | Usage |
|---|---|
| **C++17** | Application implementation |
| **OOP** | `User`, `Bank`, and `Database` classes; encapsulation |
| **STL** | `unordered_map`, `vector`, strings |
| **SQLite3** | Durable relational persistence |
| **SQL** | Account and transaction data model |
| **ACID transactions** | Atomic account mutations and transaction logging |
| **Concurrency control** | `BEGIN IMMEDIATE`, SQLite writer locking, busy timeout |
| **WAL** | SQLite write-ahead logging mode |
| **Integer minor units** | Monetary values stored as cents/paise in SQLite |
| **CSV** | Tabular account export |
| **Catch2** | Unit and integration testing |
| **Make** | Build automation |
| **Input validation** | Safe command-line interaction |

The project uses [nlohmann/json](https://github.com/nlohmann/json) as a JSON dependency for the `User` model's serialization helpers, while **SQLite is the active persistence layer used by `Bank`**.

---

## ⚙️ Installation & Setup

### Prerequisites

- A C++17-compatible compiler:
  - GCC
  - Clang
  - MSVC
- GNU Make
- SQLite3 development library and headers
- The repository's JSON header at `include/json.hpp`
- Catch2 single-header dependency at `tests/catch.hpp`

### macOS

If SQLite is not already available through the system/toolchain, install it with your preferred package manager and ensure `sqlite3` is visible to the compiler/linker.

### Linux

Install the SQLite3 development package provided by your distribution, for example:

```bash
sudo apt install g++ make libsqlite3-dev
```

### Clone the repository

```bash
git clone https://github.com/ish4722/Bank-Management-System.git
cd Bank-Management-System
```

### Build

```bash
make
```

The Makefile links the application against SQLite3 and pthreads.

### Run

```bash
make run
```

The application creates/uses the SQLite database at:

```text
data/bank.db
```

SQLite may additionally create:

```text
data/bank.db-wal
data/bank.db-shm
```

while WAL mode is active. These runtime files are ignored by Git.

### Run tests

```bash
make test
```

The test suite includes database persistence, atomicity, and concurrent-withdrawal tests in addition to the existing user-level tests.

### Clean build artifacts and runtime databases

```bash
make clean
```

### Manual compilation

```bash
g++ -std=c++17 -Wall -Wextra -I./include \
    main.cpp src/user.cpp src/bank.cpp src/database.cpp \
    -o bank -lsqlite3 -pthread
```

Then run:

```bash
./bank
```

---

## 📖 Usage Guide

When the application starts, `Bank` opens the SQLite database, ensures the schema exists, and loads accounts and transaction history into memory.

```text
==== WELCOME TO BANK MANAGEMENT SYSTEM ====
1. Create Account
2. Display Account
3. Modify Account
4. Delete Account
5. Deposit Money
6. Withdraw Money
7. View Transaction History
8. Export Accounts to CSV
9. Exit
```

### 1️⃣ Create Account

1. Select `1`.
2. Enter the account holder's name.
3. Select:
   - `0` → Savings
   - `1` → Current
4. The system generates a unique 10-digit account number.
5. The account is inserted into SQLite inside a write transaction.

New accounts start with a balance of `0.00` and an empty transaction history.

### 2️⃣ Display Account

Displays:

- Account number
- Name
- Balance
- Account type

### 3️⃣ Modify Account

Allows the account holder's name and account type to be changed. The account number and balance remain unchanged.

The update is committed through SQLite as a transaction.

### 4️⃣ Delete Account

Deletes the account from SQLite. Its transaction history is automatically deleted through the foreign-key relationship with `ON DELETE CASCADE`.

### 5️⃣ Deposit Money

A successful deposit performs the following database sequence:

```text
BEGIN IMMEDIATE
     │
     ▼
UPDATE accounts
SET balance = balance + amount
     │
     ▼
INSERT transaction record
     │
     ▼
COMMIT
```

The database stores the amount and resulting balance as integer cents/paise rather than binary floating-point values.

### 6️⃣ Withdraw Money

A withdrawal is accepted only when:

```text
amount > 0
AND
balance >= amount
```

The balance check is performed directly in the SQL `UPDATE`:

```sql
UPDATE accounts
SET balance_cents = balance_cents - ?
WHERE account_number = ?
  AND balance_cents >= ?;
```

This matters under concurrency: the application does not perform a vulnerable `SELECT balance → check in C++ → UPDATE` sequence. The sufficient-balance condition is part of the database write itself.

The successful balance update and transaction-history insert are committed together. A failed withdrawal creates no transaction record and leaves the balance unchanged.

### 7️⃣ View Transaction History

Enter an account number to display its transaction history:

```text
===== TRANSACTION HISTORY =====
Timestamp             Type           Amount         Balance After
-------------------------------------------------------------------
2026-08-29 22:15:10   Deposit        500.00         1500.00
2026-08-29 22:18:32   Withdrawal     200.00         1300.00
```

The history is stored in the SQLite `transactions` table and survives application restarts.

### 8️⃣ Export Accounts to CSV

Exports the current account state to:

```text
data/accounts.csv
```

The CSV contains account number, name, balance, and account type. Transaction history remains in SQLite.

### 9️⃣ Exit

Select `9` to exit the application.

---

## 💳 Transaction History Design

Each successful monetary operation creates one `Transaction` object in memory and one corresponding row in SQLite:

```cpp
struct Transaction {
    TransactionType type;
    double amount;
    double balanceAfter;
    std::string timestamp;
};
```

The durable representation is normalized into the `transactions` table:

```text
transactions
┌────────────┬────────────────┬──────────────┬─────────────────────┐
│ account    │ type           │ amount       │ balance_after       │
├────────────┼────────────────┼──────────────┼─────────────────────┤
│ 4829137461 │ Deposit        │ 50000        │ 150000              │
│ 4829137461 │ Withdrawal     │ 20000        │ 130000              │
└────────────┴────────────────┴──────────────┴─────────────────────┘
                 values represented in cents/paise
```

### Why store `balance_after`?

It provides a historical snapshot of the account balance immediately after each successful transaction:

```text
Transaction 1: +500  → balance 1500
Transaction 2: -200  → balance 1300
Transaction 3: +100  → balance 1400
```

This makes the transaction history useful for display and auditing while keeping the current balance separately available in `accounts`.

---

## 🔐 ACID & Concurrency Design

The SQLite implementation is the main architectural upgrade of the project.

### Atomicity

A monetary operation consists of multiple database changes:

1. Update account balance.
2. Insert transaction history.

These operations execute inside one transaction:

```text
BEGIN IMMEDIATE
     │
     ├── UPDATE accounts
     │
     ├── INSERT transactions
     │
     └── COMMIT
```

If any statement fails:

```text
ROLLBACK
```

so the partial state is not persisted.

### Consistency

The schema enforces important invariants at the database layer:

- Account numbers are unique primary keys.
- Account names cannot be empty.
- Balances cannot become negative.
- Account types are restricted to `Savings` or `Current`.
- Transaction amounts must be positive.
- Transaction balances cannot be negative.
- Transactions must reference an existing account.

### Isolation / concurrency control

Write operations use:

```sql
BEGIN IMMEDIATE TRANSACTION;
```

This acquires SQLite's write reservation before the balance-changing sequence. Competing writers therefore cannot simultaneously modify the same database state as if they were independent read-modify-write operations.

Withdrawals additionally enforce the balance invariant directly in SQL:

```sql
WHERE account_number = ?
  AND balance_cents >= ?
```

Consequently, if two concurrent requests each attempt to withdraw the entire available balance, **at most one can succeed**.

The database connection also configures a busy timeout so a writer can wait briefly for a competing lock instead of immediately failing.

### Durability

After `COMMIT`, SQLite is responsible for durable persistence. The connection uses:

```sql
PRAGMA journal_mode = WAL;
PRAGMA synchronous = FULL;
```

WAL mode improves the behavior of concurrent readers/writers for a local SQLite application, while `synchronous = FULL` requests stronger durability guarantees.

> This is still a local educational system, not a production distributed banking backend. SQLite's concurrency model is appropriate for this application's scope but does not replace the architecture required for a multi-service financial platform.

---

## 🗄️ Database Schema

The database contains two primary tables.

### `accounts`

```text
accounts
├── account_number      TEXT PRIMARY KEY
├── user_name           TEXT NOT NULL
├── balance_cents       INTEGER NOT NULL
└── account_type        TEXT NOT NULL
```

The balance is stored as an integer minor unit:

```text
₹100.50 → 10050 paise
$100.50 → 10050 cents
```

This avoids storing monetary state as a binary floating-point value inside SQLite.

### `transactions`

```text
transactions
├── id                   INTEGER PRIMARY KEY
├── account_number       TEXT FOREIGN KEY
├── type                 TEXT
├── amount_cents         INTEGER
├── balance_after_cents  INTEGER
└── timestamp            TEXT
```

The relationship is:

```text
accounts 1 ──────────── * transactions
```

Deleting an account cascades to its transaction records.

An index on `(account_number, id)` makes transaction-history retrieval efficient for a specific account.

The schema is also documented in [`sql/schema.sql`](sql/schema.sql).

---

## 📁 Project Structure

```text
Bank-Management-System/
│
├── .gitignore
├── LICENSE
├── README.md
├── Makefile
├── main.cpp                    # CLI and application entry point
│
├── include/
│   ├── json.hpp                # nlohmann/json single-header library
│   ├── user.h                  # User and Transaction models
│   ├── bank.h                  # Bank/account-manager interface
│   └── database.h              # SQLite database wrapper
│
├── src/
│   ├── user.cpp                # User, transaction and JSON helpers
│   ├── bank.cpp                # Account operations and SQLite persistence
│   └── database.cpp             # SQLite connection/schema configuration
│
├── sql/
│   └── schema.sql              # Relational SQLite schema
│
├── tests/
│   ├── catch.hpp               # Catch2 single-header test framework
│   ├── user_test.cpp           # User/model tests
│   └── bank_test.cpp            # SQLite persistence, ACID, concurrency tests
│
└── data/
    └── bank.db                 # Runtime SQLite database (gitignored)
```

---

## 🧩 Core Classes

### `User`

Represents a single bank account and owns its in-memory transaction history.

### `Transaction`

Represents one successful deposit or withdrawal with its type, amount, resulting balance, and timestamp.

### `Database`

Provides the SQLite connection and low-level database operations. It owns the `sqlite3*` handle and initializes the database configuration/schema.

### `Bank`

Coordinates account operations and persistence. It owns:

```cpp
std::unordered_map<std::string, User> accounts;
Database database;
```

Responsibilities include:

- Loading accounts from SQLite
- Creating accounts
- Finding accounts
- Depositing and withdrawing
- Modifying accounts
- Deleting accounts
- Exporting CSV
- Managing database transactions
- Reloading the in-memory representation after committed writes

---

## 🔢 Account Number Generation

Account numbers are generated as random 10-digit identifiers.

```text
Generate candidate
      │
      ▼
Insert into SQLite
      │
      ├── UNIQUE constraint violation → retry
      │
      ▼
    COMMIT
```

The database's primary-key constraint provides the final uniqueness guarantee rather than relying only on the process-local `unordered_map`.

For a production financial system, a dedicated durable identifier strategy would still be preferable.

---

## 🧪 Testing

The project uses Catch2 for automated testing.

Tests cover the original user/model behavior as well as the database-backed implementation.

### Database persistence

Verifies that:

- An account survives destruction of one `Bank` instance.
- A new `Bank` instance can reload the account from SQLite.
- Balance is restored correctly.
- Transaction history is restored correctly.

### Concurrency

Two threads open independent `Bank` instances against the same SQLite database and simultaneously attempt to withdraw the entire available balance.

The test verifies:

```text
100 balance
   │
   ├── Thread A → withdraw 100 → SUCCESS
   │
   └── Thread B → withdraw 100 → FAILURE

Final balance = 0
Successful withdrawals = 1
```

This demonstrates that the database transaction/locking strategy prevents double spending in this scenario.

### Atomicity

A withdrawal larger than the current balance is rejected without changing the account balance or inserting a transaction record.

Run all tests with:

```bash
make test
```

---

## ⏱️ Complexity

Let `N` be the number of accounts and `T` the number of transactions for one account.

| Operation | Application / DB behavior |
|---|---:|
| Find account in memory | **O(1)** average |
| Insert account | **O(log N)** database index behavior, plus reload cost |
| Delete account | **O(log N)** database index behavior, plus reload cost |
| Deposit / Withdraw SQL update | **O(log N)** indexed account lookup |
| Append transaction | **O(log T)** index maintenance |
| Load accounts | **O(N + total transactions)** |
| Load one account's history | **O(T)** result traversal |
| Export CSV | **O(N)** |

The in-memory `unordered_map` still provides average O(1) account lookup for CLI reads. Database-backed mutations have database/index overhead, but unlike the previous whole-file JSON approach they update only the affected relational rows.

---

## 🛡️ Validation & Error Handling

The application validates:

- Menu choices
- Account types
- Positive transaction amounts
- Sufficient balance before withdrawal
- Empty account names
- Missing accounts
- SQL statement preparation/execution
- Transaction commit/rollback failures
- CSV output availability

Failed database operations roll back the active transaction where appropriate.

Prepared SQL statements are used with bound parameters for user-provided values, avoiding string concatenation for account names and amounts.

---

## 🔐 Production Limitations

This project intentionally remains a local educational application. A real banking system would additionally require:

- Authentication and authorization
- Secure credential/PIN handling
- Encryption in transit and at rest
- Database access control
- Double-entry accounting / ledger semantics
- Immutable transaction identifiers
- Idempotency for retried requests
- Stronger audit logging
- Role-based access control
- Backups and disaster recovery
- Monitoring and observability
- Distributed concurrency/coordination where applicable
- Comprehensive integration and failure testing

SQLite provides ACID transactions and local concurrency control for this project, but it should not be presented as equivalent to the architecture of a production banking platform.

---

## 🚀 Future Improvements

Potential extensions include:

1. **Database migrations**
   - Versioned schema migrations
   - Seed/demo data

2. **Authentication**
   - Login/PIN
   - Role-based permissions

3. **Stronger ledger model**
   - Immutable transaction IDs
   - Double-entry accounting
   - Idempotency keys

4. **Advanced reporting**
   - Statements by date range
   - Deposits vs withdrawals
   - Monthly summaries

5. **REST API**
   - Separate frontend and backend
   - HTTP/JSON interface

6. **Threaded service layer**
   - Connection-per-worker strategy
   - Request-level transaction boundaries

7. **Stronger testing**
   - More concurrent workloads
   - Failure-injection tests
   - Database migration tests
   - Property-based tests

8. **Production database**
   - PostgreSQL/MySQL for a multi-user service where appropriate

---

## 🤝 Contribution Guidelines

1. Fork the repository.
2. Create a feature branch.
3. Keep database changes and schema changes synchronized.
4. Add or update tests for behavioral changes.
5. Run:

```bash
make test
```

6. Open a pull request describing the change and its database/concurrency implications where applicable.

---

## 📄 License

This project is licensed under the MIT License. See [`LICENSE`](LICENSE) for details.
