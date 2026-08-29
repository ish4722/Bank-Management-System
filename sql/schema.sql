-- SQLite schema for the Bank Management System.
-- Monetary values are stored as integer paise/cents to avoid binary floating-point
-- rounding in the persistence layer.

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

-- Writes use BEGIN IMMEDIATE so the writer lock is acquired before the
-- balance read/update sequence. Deposits/withdrawals then update the balance
-- and append the transaction record in one COMMIT/ROLLBACK unit.
