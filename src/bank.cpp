#include "bank.h"

#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <random>
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <utility>

using namespace std;

namespace {
string currentTimestamp() {
    const time_t now = time(nullptr);
    tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
    return buffer;
}

long long toCents(double amount) {
    if (!isfinite(amount) || amount <= 0.0) return 0;
    const long double cents = static_cast<long double>(amount) * 100.0L;
    if (cents > static_cast<long double>(numeric_limits<long long>::max())) return 0;
    return llround(cents);
}

string typeToString(Type type) {
    return accountTypeMap.at(type);
}

Type typeFromString(const string& value) {
    return value == "Current" ? Type::CURRENT : Type::SAVINGS;
}

TransactionType transactionTypeFromString(const string& value) {
    return value == "Withdrawal" ? TransactionType::WITHDRAWAL : TransactionType::DEPOSIT;
}

string makeAccountNumber() {
    static mt19937_64 rng(random_device{}());
    uniform_int_distribution<long long> dist(1000000000LL, 9999999999LL);
    return to_string(dist(rng));
}

bool prepare(sqlite3* db, const char* sql, sqlite3_stmt** stmt) {
    return sqlite3_prepare_v2(db, sql, -1, stmt, nullptr) == SQLITE_OK;
}
}

Bank::Bank(const string& databasePath) : database(databasePath) {
    if (!database.initializeSchema()) {
        throw runtime_error("Could not initialize SQLite schema.");
    }
}

bool Bank::beginWriteTransaction() {
    return database.execute("BEGIN IMMEDIATE TRANSACTION;");
}

bool Bank::commitTransaction() {
    return database.execute("COMMIT;");
}

void Bank::rollbackTransaction() {
    database.execute("ROLLBACK;");
}

bool Bank::load() {
    return reloadFromDatabase();
}

bool Bank::reloadFromDatabase() {
    accounts.clear();
    sqlite3* db = database.handle();
    sqlite3_stmt* stmt = nullptr;

    if (!prepare(db,
                 "SELECT account_number, user_name, balance_cents, account_type "
                 "FROM accounts ORDER BY account_number;",
                 &stmt)) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const string accountNumber(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        const string name(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        const double balance = sqlite3_column_int64(stmt, 2) / 100.0;
        const string type(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));

        User user(accountNumber, name, balance, typeFromString(type));

        sqlite3_stmt* txStmt = nullptr;
        if (!prepare(db,
                     "SELECT type, amount_cents, balance_after_cents, timestamp "
                     "FROM transactions WHERE account_number = ? ORDER BY id;",
                     &txStmt)) {
            sqlite3_finalize(stmt);
            accounts.clear();
            return false;
        }

        sqlite3_bind_text(txStmt, 1, accountNumber.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(txStmt) == SQLITE_ROW) {
            Transaction tx{
                transactionTypeFromString(reinterpret_cast<const char*>(sqlite3_column_text(txStmt, 0))),
                sqlite3_column_int64(txStmt, 1) / 100.0,
                sqlite3_column_int64(txStmt, 2) / 100.0,
                reinterpret_cast<const char*>(sqlite3_column_text(txStmt, 3))
            };
            user.transactions.push_back(std::move(tx));
        }
        sqlite3_finalize(txStmt);
        accounts.emplace(accountNumber, std::move(user));
    }

    const bool ok = sqlite3_errcode(db) == SQLITE_OK;
    sqlite3_finalize(stmt);
    return ok;
}

bool Bank::save() const {
    // SQLite writes are committed by the individual ACID operations below.
    // Kept for API compatibility with the original JSON-backed implementation.
    return database.handle() != nullptr;
}

bool Bank::createAccount(const string& name, Type type, string& accountNumber) {
    if (name.empty()) return false;

    sqlite3* db = database.handle();
    for (int attempt = 0; attempt < 10; ++attempt) {
        accountNumber = makeAccountNumber();

        if (!beginWriteTransaction()) return false;

        sqlite3_stmt* stmt = nullptr;
        const char* sql =
            "INSERT INTO accounts(account_number, user_name, balance_cents, account_type) "
            "VALUES (?, ?, 0, ?);";

        if (!prepare(db, sql, &stmt)) {
            rollbackTransaction();
            return false;
        }

        sqlite3_bind_text(stmt, 1, accountNumber.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);
        const string typeName = typeToString(type);
        sqlite3_bind_text(stmt, 3, typeName.c_str(), -1, SQLITE_TRANSIENT);

        const int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (rc == SQLITE_CONSTRAINT) {
            rollbackTransaction();
            continue;
        }
        if (rc != SQLITE_DONE || !commitTransaction()) {
            rollbackTransaction();
            return false;
        }

        return reloadFromDatabase();
    }

    return false;
}

User* Bank::findAccount(const string& accountNumber) {
    auto it = accounts.find(accountNumber);
    return it == accounts.end() ? nullptr : &it->second;
}

const User* Bank::findAccount(const string& accountNumber) const {
    auto it = accounts.find(accountNumber);
    return it == accounts.end() ? nullptr : &it->second;
}

bool Bank::deposit(const string& accountNumber, double amount) {
    const long long cents = toCents(amount);
    if (cents <= 0) return false;

    sqlite3* db = database.handle();
    if (!beginWriteTransaction()) return false;

    sqlite3_stmt* update = nullptr;
    if (!prepare(db,
                 "UPDATE accounts SET balance_cents = balance_cents + ? "
                 "WHERE account_number = ?;",
                 &update)) {
        rollbackTransaction();
        return false;
    }

    sqlite3_bind_int64(update, 1, cents);
    sqlite3_bind_text(update, 2, accountNumber.c_str(), -1, SQLITE_TRANSIENT);
    const int updateRc = sqlite3_step(update);
    const int changed = sqlite3_changes(db);
    sqlite3_finalize(update);

    if (updateRc != SQLITE_DONE || changed != 1) {
        rollbackTransaction();
        return false;
    }

    sqlite3_stmt* balanceStmt = nullptr;
    if (!prepare(db, "SELECT balance_cents FROM accounts WHERE account_number = ?;", &balanceStmt)) {
        rollbackTransaction();
        return false;
    }
    sqlite3_bind_text(balanceStmt, 1, accountNumber.c_str(), -1, SQLITE_TRANSIENT);
    const int balanceRc = sqlite3_step(balanceStmt);
    const long long newBalance = balanceRc == SQLITE_ROW ? sqlite3_column_int64(balanceStmt, 0) : -1;
    sqlite3_finalize(balanceStmt);

    if (balanceRc != SQLITE_ROW) {
        rollbackTransaction();
        return false;
    }

    sqlite3_stmt* insert = nullptr;
    if (!prepare(db,
                 "INSERT INTO transactions(account_number, type, amount_cents, balance_after_cents, timestamp) "
                 "VALUES (?, 'Deposit', ?, ?, ?);",
                 &insert)) {
        rollbackTransaction();
        return false;
    }

    const string timestamp = currentTimestamp();
    sqlite3_bind_text(insert, 1, accountNumber.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(insert, 2, cents);
    sqlite3_bind_int64(insert, 3, newBalance);
    sqlite3_bind_text(insert, 4, timestamp.c_str(), -1, SQLITE_TRANSIENT);
    const int insertRc = sqlite3_step(insert);
    sqlite3_finalize(insert);

    if (insertRc != SQLITE_DONE || !commitTransaction()) {
        rollbackTransaction();
        return false;
    }

    return reloadFromDatabase();
}

bool Bank::withdraw(const string& accountNumber, double amount) {
    const long long cents = toCents(amount);
    if (cents <= 0) return false;

    sqlite3* db = database.handle();
    if (!beginWriteTransaction()) return false;

    // The balance predicate is evaluated while the write transaction owns
    // the writer lock. This prevents two concurrent withdrawals from both
    // observing and spending the same balance.
    sqlite3_stmt* update = nullptr;
    if (!prepare(db,
                 "UPDATE accounts SET balance_cents = balance_cents - ? "
                 "WHERE account_number = ? AND balance_cents >= ?;",
                 &update)) {
        rollbackTransaction();
        return false;
    }

    sqlite3_bind_int64(update, 1, cents);
    sqlite3_bind_text(update, 2, accountNumber.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(update, 3, cents);
    const int updateRc = sqlite3_step(update);
    const int changed = sqlite3_changes(db);
    sqlite3_finalize(update);

    if (updateRc != SQLITE_DONE || changed != 1) {
        rollbackTransaction();
        return false;
    }

    sqlite3_stmt* balanceStmt = nullptr;
    if (!prepare(db, "SELECT balance_cents FROM accounts WHERE account_number = ?;", &balanceStmt)) {
        rollbackTransaction();
        return false;
    }
    sqlite3_bind_text(balanceStmt, 1, accountNumber.c_str(), -1, SQLITE_TRANSIENT);
    const int balanceRc = sqlite3_step(balanceStmt);
    const long long newBalance = balanceRc == SQLITE_ROW ? sqlite3_column_int64(balanceStmt, 0) : -1;
    sqlite3_finalize(balanceStmt);

    if (balanceRc != SQLITE_ROW) {
        rollbackTransaction();
        return false;
    }

    sqlite3_stmt* insert = nullptr;
    if (!prepare(db,
                 "INSERT INTO transactions(account_number, type, amount_cents, balance_after_cents, timestamp) "
                 "VALUES (?, 'Withdrawal', ?, ?, ?);",
                 &insert)) {
        rollbackTransaction();
        return false;
    }

    const string timestamp = currentTimestamp();
    sqlite3_bind_text(insert, 1, accountNumber.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(insert, 2, cents);
    sqlite3_bind_int64(insert, 3, newBalance);
    sqlite3_bind_text(insert, 4, timestamp.c_str(), -1, SQLITE_TRANSIENT);
    const int insertRc = sqlite3_step(insert);
    sqlite3_finalize(insert);

    if (insertRc != SQLITE_DONE || !commitTransaction()) {
        rollbackTransaction();
        return false;
    }

    return reloadFromDatabase();
}

bool Bank::modifyAccount(const string& accountNumber, const string& name, Type type) {
    if (name.empty()) return false;
    if (!beginWriteTransaction()) return false;

    sqlite3_stmt* stmt = nullptr;
    if (!prepare(database.handle(),
                 "UPDATE accounts SET user_name = ?, account_type = ? WHERE account_number = ?;",
                 &stmt)) {
        rollbackTransaction();
        return false;
    }

    const string typeName = typeToString(type);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, typeName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, accountNumber.c_str(), -1, SQLITE_TRANSIENT);

    const int rc = sqlite3_step(stmt);
    const int changed = sqlite3_changes(database.handle());
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE || changed != 1 || !commitTransaction()) {
        rollbackTransaction();
        return false;
    }

    return reloadFromDatabase();
}

bool Bank::deleteAccount(const string& accountNumber) {
    if (!beginWriteTransaction()) return false;

    sqlite3_stmt* stmt = nullptr;
    if (!prepare(database.handle(), "DELETE FROM accounts WHERE account_number = ?;", &stmt)) {
        rollbackTransaction();
        return false;
    }

    sqlite3_bind_text(stmt, 1, accountNumber.c_str(), -1, SQLITE_TRANSIENT);
    const int rc = sqlite3_step(stmt);
    const int changed = sqlite3_changes(database.handle());
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE || changed != 1 || !commitTransaction()) {
        rollbackTransaction();
        return false;
    }

    return reloadFromDatabase();
}

bool Bank::exportToCSV(const string& path) const {
    ofstream outFile(path);
    if (!outFile) return false;

    outFile << "Account Number,Name,Balance,Type\n";
    for (const auto& [accountNumber, user] : accounts) {
        outFile << accountNumber << ','
                << user.getUserName() << ','
                << fixed << setprecision(2) << user.getBalance() << ','
                << accountTypeMap.at(user.getAccountType()) << '\n';
    }

    return static_cast<bool>(outFile);
}

size_t Bank::size() const {
    return accounts.size();
}
