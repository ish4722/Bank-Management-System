-- Relational schema for the Bank Management System.
-- PostgreSQL-oriented SQL. The application can use the same model with
-- equivalent types/syntax in MySQL/SQLite.

CREATE TABLE IF NOT EXISTS accounts (
    account_id BIGSERIAL PRIMARY KEY,
    account_number VARCHAR(20) NOT NULL UNIQUE,
    user_name VARCHAR(100) NOT NULL,
    account_type VARCHAR(20) NOT NULL CHECK (account_type IN ('Savings', 'Current')),
    balance NUMERIC(19,4) NOT NULL DEFAULT 0 CHECK (balance >= 0),
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS transactions (
    transaction_id BIGSERIAL PRIMARY KEY,
    account_id BIGINT NOT NULL REFERENCES accounts(account_id) ON DELETE CASCADE,
    transaction_type VARCHAR(20) NOT NULL CHECK (transaction_type IN ('Deposit', 'Withdrawal')),
    amount NUMERIC(19,4) NOT NULL CHECK (amount > 0),
    balance_after NUMERIC(19,4) NOT NULL CHECK (balance_after >= 0),
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_accounts_account_number
    ON accounts(account_number);

CREATE INDEX IF NOT EXISTS idx_transactions_account_id_created_at
    ON transactions(account_id, created_at DESC);

-- Atomic withdrawal pattern. The balance check happens inside the UPDATE,
-- so two concurrent withdrawals cannot both spend the same funds.
-- The application should execute this UPDATE and its corresponding INSERT
-- into transactions in the same database transaction.
--
-- UPDATE accounts
-- SET balance = balance - :amount,
--     updated_at = CURRENT_TIMESTAMP
-- WHERE account_id = :account_id
--   AND balance >= :amount;
