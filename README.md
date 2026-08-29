# Bank Management System

![Made with C++](https://img.shields.io/badge/Made%20with-C%2B%2B17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge)

A modular **C++17 console-based Bank Management System** for managing savings and current accounts. The project demonstrates object-oriented programming, STL containers, file-based persistence, JSON serialization, CSV export, input validation, persistent transaction history, and unit testing.

> **Note:** This is an educational/portfolio project. It is not intended to represent a production banking system and does not provide authentication, encryption, database transactions, or other controls required for real financial software.

---

## 📌 Features

- ✅ Create Savings and Current accounts
- ✅ Generate collision-checked account numbers
- ✅ View account details
- ✅ Modify account name and type
- ✅ Deposit money
- ✅ Withdraw money with balance validation
- ✅ Persistent transaction history
- ✅ View transaction history from the CLI
- ✅ Store transaction timestamp, type, amount, and resulting balance
- ✅ Delete accounts
- ✅ Persist account and transaction data in JSON
- ✅ Export account data to CSV
- ✅ O(1) average account lookup using `std::unordered_map`
- ✅ Centralized account management through a `Bank` class
- ✅ Input validation for menu choices, account types, names, and amounts
- ✅ Unit tests using Catch2
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
                    │       Bank          │
                    │ Account Management  │
                    └──────────┬──────────┘
                               │
                    ┌──────────┴──────────┐
                    ▼                     ▼
             unordered_map            Persistence
             <AccountNo, User>             │
                    │                 ┌────┴────┐
                    ▼                 ▼         ▼
                  User              JSON      CSV
                    │
                    ▼
              Transactions
                    │
             ┌──────┴──────┐
             ▼             ▼
          Deposit       Withdrawal
```

### Design goals

- **Single source of truth:** account state is owned by `Bank`; there is no duplicated global/local account collection.
- **Separation of concerns:** `User` manages account state and transactions, while `Bank` manages the collection and persistence workflow.
- **Efficient lookup:** account numbers are keys in an `unordered_map`, giving average O(1) lookup.
- **Persistent audit trail:** successful deposits and withdrawals append immutable transaction records to the account's history.
- **Testability:** core operations do not require console input, making them straightforward to unit test.

---

## 🛠️ Technologies & Concepts

| Technology / Concept | Usage |
|---|---|
| **C++17** | Application implementation |
| **OOP** | `User` and `Bank` classes, encapsulation |
| **STL** | `unordered_map`, `vector`, strings |
| **JSON** | Persistent account and transaction storage |
| **CSV** | Tabular account export |
| **Catch2** | Unit testing |
| **Make** | Build automation |
| **File I/O** | Reading/writing persistent data |
| **Input validation** | Safe command-line interaction |
| **Transaction history** | Timestamped deposit/withdrawal records |

The project uses the single-header [nlohmann/json](https://github.com/nlohmann/json) library for JSON serialization.

---

## ⚙️ Installation & Setup

### Prerequisites

- A C++17-compatible compiler:
  - GCC
  - Clang
  - MSVC
- GNU Make for the Makefile workflow
- The repository's JSON header at `include/json.hpp`
- Catch2 single-header dependency at `tests/catch.hpp`

### Clone the repository

```bash
git clone https://github.com/ish4722/Bank-Management-System.git
cd Bank-Management-System
```

### Build

```bash
make
```

### Run

```bash
make run
```

### Run tests

```bash
make test
```

### Clean build artifacts

```bash
make clean
```

### Manual compilation

```bash
g++ -std=c++17 -Wall -Wextra -I./include \
    main.cpp src/user.cpp src/bank.cpp \
    -o bank
```

Then run:

```bash
./bank
```

---

## 📖 Usage Guide

When the application starts, it loads existing accounts and their transaction histories from `data/accounts.json`.

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
4. The system generates a unique account number.
5. The new account is persisted to JSON.

New accounts start with a balance of `0.00` and an empty transaction history.

### 2️⃣ Display Account

Displays:

- Account number
- Name
- Balance
- Account type

### 3️⃣ Modify Account

Allows the account holder's name and account type to be changed. The account number and balance remain unchanged.

### 4️⃣ Delete Account

Removes the account and its stored transaction history from the active JSON data.

### 5️⃣ Deposit Money

A successful deposit:

1. Validates that the amount is positive.
2. Increases the account balance.
3. Creates a transaction record.
4. Stores the resulting balance in that record.
5. Records the current timestamp.
6. Persists the updated account to JSON.

Example transaction:

```json
{
    "type": "Deposit",
    "amount": 500.0,
    "balance_after": 1500.0,
    "timestamp": "2026-08-29 22:15:10"
}
```

### 6️⃣ Withdraw Money

A successful withdrawal requires:

```text
amount > 0
AND
amount <= current balance
```

After a successful withdrawal, the system records the transaction and persists the updated balance and history.

Failed withdrawals do **not** create transaction records.

### 7️⃣ View Transaction History

Enter an account number to display its transaction history:

```text
===== TRANSACTION HISTORY =====
Timestamp             Type           Amount         Balance After
-------------------------------------------------------------------
2026-08-29 22:15:10   Deposit        500.00         1500.00
2026-08-29 22:18:32   Withdrawal     200.00         1300.00
```

The history is stored with the account and survives application restarts.

### 8️⃣ Export Accounts to CSV

Exports account information to:

```text
data/accounts.csv
```

The CSV export contains the current account state. Transaction history remains stored in JSON.

### 9️⃣ Exit

Select `9` to exit the application.

---

## 💳 Transaction History Design

Each successful monetary operation creates one `Transaction` object:

```cpp
struct Transaction {
    TransactionType type;
    double amount;
    double balanceAfter;
    std::string timestamp;
};
```

The transaction is appended only **after** the account operation succeeds.

```text
Deposit / Withdrawal request
          │
          ▼
     Validate amount
          │
       ┌──┴──┐
       │     │
     Invalid Valid
       │     │
       ▼     ▼
      Fail  Update balance
                │
                ▼
          Create Transaction
                │
                ├── Type
                ├── Amount
                ├── Balance After
                └── Timestamp
                │
                ▼
            save() → JSON
```

### Why store `balanceAfter`?

Storing the resulting balance makes each transaction self-describing and provides a useful historical snapshot:

```text
Transaction 1: +500  → balance 1500
Transaction 2: -200  → balance 1300
Transaction 3: +100  → balance 1400
```

This is useful for displaying account history and auditing the sequence of successful operations.

> For a production financial ledger, transactions should be immutable, uniquely identified, durably stored, and protected by database transactions and audit controls.

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
│   └── bank.h                  # Bank/account-manager interface
│
├── src/
│   ├── user.cpp                # User, transaction and JSON logic
│   └── bank.cpp                # Account collection and persistence logic
│
├── tests/
│   ├── catch.hpp               # Catch2 single-header test framework
│   └── user_test.cpp           # Unit tests
│
└── data/
    ├── accounts.json           # Persistent account + transaction data
    └── accounts.csv            # Generated account export
```

---

## 🧩 Core Classes

### `User`

Represents a single bank account and owns its transaction history.

Private state:

```cpp
std::string account_number;
std::string user_name;
double account_balance;
Type account_type;
std::vector<Transaction> transactions;
```

### `Transaction`

Represents one successful deposit or withdrawal:

```cpp
TransactionType type;
double amount;
double balanceAfter;
std::string timestamp;
```

### `Bank`

Owns and manages all accounts:

```cpp
std::unordered_map<std::string, User> accounts;
```

Responsibilities include:

- Loading accounts from JSON
- Saving accounts to JSON
- Creating accounts
- Finding accounts
- Depositing and withdrawing
- Modifying accounts
- Deleting accounts
- Exporting CSV

---

## 🔢 Account Number Generation

Account numbers are generated by `Bank` rather than relying on a process-local static counter.

The implementation generates a random 10-digit identifier and checks the existing account map before accepting it:

```text
Generate candidate
      │
      ▼
Already exists?
   │       │
  YES      NO
   │       │
   └─ retry ▼
          use ID
```

This avoids the restart/deletion problems associated with a simple in-memory counter.

For a production financial system, a durable unique identifier service would be preferable.

---

## 💾 Data Persistence

### JSON

Account state and transaction history are persisted together:

```json
[
    {
        "account_number": "4829137461",
        "user_name": "John Doe",
        "account_balance": 1300.0,
        "account_type": "Savings",
        "transactions": [
            {
                "type": "Deposit",
                "amount": 1500.0,
                "balance_after": 1500.0,
                "timestamp": "2026-08-29 22:15:10"
            },
            {
                "type": "Withdrawal",
                "amount": 200.0,
                "balance_after": 1300.0,
                "timestamp": "2026-08-29 22:18:32"
            }
        ]
    }
]
```

### Persistence flow

```text
Application
     │
     ▼
 Bank / unordered_map
     │
     ▼
 save()
     │
     ▼
 data/accounts.json
```

At startup:

```text
 data/accounts.json
        │
        ▼
      load()
        │
        ▼
 Bank / unordered_map
```

Older account JSON files that do not contain a `transactions` field are still loadable; such accounts start with an empty history.

### CSV

CSV is used for convenient tabular account export:

```csv
Account Number,Name,Balance,Type
4829137461,John Doe,1300.00,Savings
```

Transaction history remains available through JSON and the application's transaction-history menu.

---

## 🧪 Testing

The project uses Catch2 for unit testing.

Tests cover:

- Valid deposits
- Invalid deposits
- Valid withdrawals
- Insufficient balance
- Invalid withdrawals
- Account type serialization
- Account field updates
- JSON round-trip
- Transaction creation
- Transaction type and amount
- Resulting balance after transaction
- Transaction history persistence through JSON
- Timestamp creation

Run the tests with:

```bash
make test
```

---

## ⏱️ Complexity

Let `N` be the number of accounts and `T` the number of transactions for one account.

| Operation | Complexity |
|---|---:|
| Find account | **O(1)** average |
| Insert account | **O(1)** average |
| Delete account | **O(1)** average |
| Deposit / Withdraw in memory | **O(1)** |
| Append transaction | **O(1)** amortized |
| View transaction history | **O(T)** |
| Export CSV | **O(N)** |
| Load JSON | **O(N + total transactions)** |
| Save JSON | **O(N + total transactions)** |

Although account lookup is efficient, each successful mutation rewrites the complete JSON file. Therefore persistence remains proportional to the total stored data.

For large-scale systems, a database with indexes and transactional updates would be more appropriate.

---

## 🛡️ Validation & Error Handling

The application validates:

- Menu choices
- Account types
- Positive transaction amounts
- Sufficient balance before withdrawal
- Empty account names
- Missing accounts
- JSON file availability/format
- CSV file availability

Invalid deposit/withdrawal requests do not alter the balance or create transaction records.

---

## 🔐 Production Limitations

This project intentionally remains a local educational application. A real banking system would additionally require:

- Authentication and authorization
- Secure password/PIN handling
- Encryption in transit and at rest
- Database-backed persistence
- ACID transactions
- Concurrency control
- Immutable transaction IDs
- Double-entry accounting / ledger semantics
- Audit logging
- Role-based access control
- Backups and disaster recovery
- Monitoring and observability

The current JSON persistence and `double` balance representation are suitable for demonstrating C++ concepts but are not appropriate for real financial workloads.

For production monetary calculations, integer minor units (such as paise/cents) or a suitable decimal fixed-point representation should be preferred over binary floating-point values.

---

## 🚀 Future Improvements

Potential extensions include:

1. **Database persistence**
   - SQLite for a local application
   - PostgreSQL/MySQL for a multi-user service

2. **Authentication**
   - Login/PIN
   - Role-based permissions

3. **Transaction identifiers**
   - Unique transaction ID
   - Search/filter transactions

4. **Advanced reporting**
   - Statements by date range
   - Deposits vs withdrawals
   - Monthly summaries

5. **REST API**
   - Separate frontend and backend
   - HTTP/JSON interface

6. **Concurrency support**
   - Thread-safe account operations
   - Database transactions/locking

7. **Better money representation**
   - Store amounts as integer paise/cents instead of `double`

8. **Stronger testing**
   - Persistence tests
   - Bank-level integration tests
   - Input-validation tests
   - Edge-case and property-based tests

---

## 🤝 Contribution Guidelines

1. Fork the repository.
2. Create a feature branch.
3. Make focused changes.
4. Add/update tests where appropriate.
5. Build and test locally.
6. Open a pull request with a clear description.

Example:

```bash
git checkout -b feature/transaction-history
git add .
git commit -m "Add persistent transaction history"
git push origin feature/transaction-history
```

---

## 📜 License

This project is licensed under the **MIT License**. See [`LICENSE`](LICENSE) for details.

The project includes third-party libraries with their own licensing information. In particular, `include/json.hpp` is provided by the nlohmann/json project under the MIT License.

---

## 👨‍💻 Author

**Ishu**

GitHub: [ish4722](https://github.com/ish4722)

Repository: [Bank-Management-System](https://github.com/ish4722/Bank-Management-System)
