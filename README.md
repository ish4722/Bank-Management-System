# Bank Management System

![Made with C++](https://img.shields.io/badge/Made%20with-C%2B%2B17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge)

A modular **C++17 console-based Bank Management System** for managing savings and current accounts. The project demonstrates object-oriented programming, STL containers, file-based persistence, JSON serialization, CSV export, input validation, and unit testing.

> **Note:** This is an educational/portfolio project. It is not intended to represent a production banking system and does not provide authentication, encryption, database transactions, or other controls required for real financial software.

---

## 📌 Features

- ✅ Create Savings and Current accounts
- ✅ Generate collision-checked account numbers
- ✅ View account details
- ✅ Modify account name and type
- ✅ Deposit money
- ✅ Withdraw money with balance validation
- ✅ Delete accounts
- ✅ Persist account data in JSON
- ✅ Export account data to CSV
- ✅ O(1) average account lookup using `std::unordered_map`
- ✅ Centralized account management through a `Bank` class
- ✅ Input validation for menu choices, account types, names, and amounts
- ✅ Unit tests using Catch2
- ✅ C++17 and Makefile-based build

---

## 🏗️ Architecture

The project separates the user/account model from account management and the command-line interface:

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
             Account State          Storage    Export
```

### Design goals

- **Single source of truth:** account state is owned by `Bank`; there is no duplicated global/local account collection.
- **Separation of concerns:** `User` manages account state and account-level operations, while `Bank` manages the collection and persistence workflow.
- **Efficient lookup:** account numbers are keys in an `unordered_map`, giving average O(1) lookup.
- **Testability:** core operations do not require console input, making them straightforward to unit test.

---

## 🛠️ Technologies & Concepts

| Technology / Concept | Usage |
|---|---|
| **C++17** | Application implementation |
| **OOP** | `User` and `Bank` classes, encapsulation |
| **STL** | `unordered_map`, strings, iterators |
| **JSON** | Persistent account storage |
| **CSV** | Tabular data export |
| **Catch2** | Unit testing |
| **Make** | Build automation |
| **File I/O** | Reading/writing persistent data |
| **Input validation** | Safe command-line interaction |

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

### Clone the repository

```bash
git clone https://github.com/ish4722/Bank-Management-System.git
cd Bank-Management-System
```

### Build with Make

```bash
make
```

### Run

```bash
make run
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

> On Windows with MinGW, use the equivalent executable name/command for your environment.

---

## 📖 Usage Guide

When the application starts, it loads existing accounts from `data/accounts.json` and displays the following menu:

```text
==== WELCOME TO BANK MANAGEMENT SYSTEM ====
1. Create Account
2. Display Account
3. Modify Account
4. Delete Account
5. Deposit Money
6. Withdraw Money
7. Export Accounts to CSV
8. Exit
```

### 1️⃣ Create Account

1. Select `1`.
2. Enter the account holder's name.
3. Select:
   - `0` → Savings
   - `1` → Current
4. The system generates a unique account number.
5. The new account is persisted to JSON.

New accounts start with a balance of `0.00`.

### 2️⃣ Display Account

1. Select `2`.
2. Enter the account number.
3. The application displays:
   - Account number
   - Name
   - Balance
   - Account type

### 3️⃣ Modify Account

1. Select `3`.
2. Enter the account number.
3. Enter the new name.
4. Select the new account type.
5. Changes are persisted immediately.

The account number and balance are not modified.

### 4️⃣ Delete Account

1. Select `4`.
2. Enter the account number.
3. The account is removed from the in-memory collection and JSON storage.

### 5️⃣ Deposit Money

1. Select `5`.
2. Enter the account number.
3. Enter a positive amount.
4. The balance is increased and persisted.

### 6️⃣ Withdraw Money

1. Select `6`.
2. Enter the account number.
3. Enter a positive amount.
4. Withdrawal succeeds only when the account has sufficient funds.

```text
amount > 0
AND
amount <= current balance
```

### 7️⃣ Export Accounts to CSV

Select `7` to export all accounts to:

```text
data/accounts.csv
```

### 8️⃣ Exit

Select `8` to exit the application.

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
│   ├── user.h                  # User/account model
│   └── bank.h                  # Bank/account-manager interface
│
├── src/
│   ├── user.cpp                # User implementation and JSON conversion
│   └── bank.cpp                # Account collection and persistence logic
│
├── tests/
│   ├── catch.hpp               # Catch2 single-header test framework
│   └── user_test.cpp           # Unit tests
│
└── data/
    ├── accounts.json           # Persistent account data
    └── accounts.csv            # Generated CSV export
```

---

## 🧩 Core Classes

### `User`

`User` represents one bank account.

Private state includes:

```cpp
std::string account_number;
std::string user_name;
double account_balance;
Type account_type;
```

The state is encapsulated and accessed through public methods such as:

```cpp
getAccountNumber()
getUserName()
getBalance()
getAccountType()
setUserName()
setAccountType()
setBalance()
deposit()
withdraw()
```

### `Bank`

`Bank` manages the complete collection of accounts:

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

This prevents `main.cpp` and the account model from maintaining separate copies of the application's account state.

---

## 🔢 Account Number Generation

Account numbers are generated by `Bank` rather than relying on a process-local static counter.

The current implementation generates a random 10-digit identifier and checks the existing account map before accepting it:

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

For a production financial system, an account-number/identifier service backed by durable storage would be preferable.

---

## 💾 Data Persistence

### JSON

The application uses JSON as its file-based persistence format.

Example:

```json
[
    {
        "account_number": "4829137461",
        "user_name": "John Doe",
        "account_balance": 1000.50,
        "account_type": "Savings"
    }
]
```

The flow is:

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

### CSV

CSV is used for convenient tabular export:

```csv
Account Number,Name,Balance,Type
4829137461,John Doe,1000.50,Savings
```

JSON is therefore used for persistence, while CSV is intended primarily for reporting/export.

---

## 🧪 Testing

The project uses Catch2 for unit testing.

Current tests cover:

- Deposit increases the balance correctly
- Successful withdrawal decreases the balance
- Withdrawal fails when funds are insufficient
- Account type is correctly serialized

Example:

```cpp
TEST_CASE("Withdraw fails with insufficient balance", "[withdraw]") {
    User u = createTestUser("Low Funds", Type::CURRENT, 50.0);
    bool result = u.withdraw(100.0);

    REQUIRE(result == false);
    REQUIRE(u.getBalance() == Approx(50.0));
}
```

The account model is deliberately separated from interactive input so these operations can be tested without simulating terminal input.

---

## ⏱️ Complexity

Let `N` be the number of accounts.

| Operation | Average Complexity |
|---|---:|
| Find account | **O(1)** |
| Insert account | **O(1)** average |
| Delete account | **O(1)** average |
| Deposit | **O(1)** average + JSON write |
| Withdraw | **O(1)** average + JSON write |
| Modify | **O(1)** average + JSON write |
| Export CSV | **O(N)** |
| Load JSON | **O(N)** |
| Save JSON | **O(N)** |

Although account lookup is now efficient, every successful mutation rewrites the complete JSON file. Therefore persistence remains **O(N)** for each mutation.

For a large-scale system, a database with indexed records and transactional updates would be more appropriate.

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

Invalid input is rejected rather than silently changing account state.

---

## 🔐 Production Limitations

This project intentionally remains a local educational application. A real banking system would additionally require:

- Authentication and authorization
- Secure password/PIN handling
- Encryption in transit and at rest
- Database-backed persistence
- ACID transactions
- Concurrency control
- Transaction IDs and immutable transaction history
- Audit logging
- Role-based access control
- Input/API security
- Reliable unique identifier generation
- Backups and disaster recovery
- Monitoring and observability

The current JSON persistence is useful for demonstrating serialization and file I/O, but should not be used as the datastore for real banking workloads.

---

## 🚀 Future Improvements

Potential extensions include:

1. **Transaction history**
   - Deposit/withdrawal records
   - Timestamp
   - Amount
   - Balance after transaction

2. **Database persistence**
   - SQLite for a local application
   - PostgreSQL/MySQL for a multi-user service

3. **Authentication**
   - Login/PIN
   - Role-based permissions

4. **REST API**
   - Separate frontend and backend
   - HTTP/JSON interface

5. **Concurrency support**
   - Thread-safe account operations
   - Database transactions/locking

6. **Stronger testing**
   - Persistence tests
   - Input-validation tests
   - Property/edge-case tests

7. **Better money representation**
   - Store monetary values as integer minor units (for example, paise/cents) instead of `double` to avoid floating-point monetary precision issues.

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
git commit -m "Add transaction history"
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
