#include "database.h"

#include <filesystem>
#include <stdexcept>

namespace {
constexpr const char* kSchema = R"SQL(
PRAGMA foreign_keys = ON;
PRAGMA journal_mode = WAL;
PRAGMA synchronous = FULL;

CREATE TABLE IF NOT EXISTS accounts (
    account_number TEXT PRIMARY KEY,
    user_name TEXT NOT NULL CHECK (length(trim(user_name)) > 0),
    balance_cents INTEGER NOT NULL DEFAULT 0 CHECK (balance_cents >= 0),
    account_type TEXT NOT NULL CHECK (account_type IN ('Savings', 'Current'))
);

CREATE TABLE IF NOT EXISTS transactions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    account_number TEXT NOT NULL,
    type TEXT NOT NULL CHECK (type IN ('Deposit', 'Withdrawal')),
    amount_cents INTEGER NOT NULL CHECK (amount_cents > 0),
    balance_after_cents INTEGER NOT NULL CHECK (balance_after_cents >= 0),
    timestamp TEXT NOT NULL,
    FOREIGN KEY (account_number) REFERENCES accounts(account_number) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_transactions_account
ON transactions(account_number, id);
)SQL";
}

Database::Database(const std::string& path) {
    std::filesystem::path dbPath(path);
    if (dbPath.has_parent_path()) std::filesystem::create_directories(dbPath.parent_path());

    if (sqlite3_open_v2(path.c_str(), &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
        std::string error = db_ ? sqlite3_errmsg(db_) : "Unable to open database";
        close();
        throw std::runtime_error(error);
    }

    sqlite3_busy_timeout(db_, 5000);
}

Database::~Database() { close(); }

bool Database::open() { return db_ != nullptr; }

void Database::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool Database::initializeSchema() { return execute(kSchema); }

bool Database::execute(const std::string& sql) const {
    char* error = nullptr;
    const int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error);
    if (error) sqlite3_free(error);
    return rc == SQLITE_OK;
}
