# Database Design and Concurrency Model

This document describes the relational DBMS architecture planned for scaling the Bank Management System beyond the current local JSON implementation.

## 1. Relational model

```text
accounts
---------
account_id PK
account_number UNIQUE
user_name
account_type
balance
created_at
updated_at
        │
        │ 1 : N
        ▼
transactions
------------
transaction_id PK
account_id FK
transaction_type
amount
balance_after
created_at
```

### Why normalize transactions?

Transaction history is a one-to-many relationship: one account can have many transactions. Keeping transactions in a separate table avoids repeatedly storing account attributes and lets the database index and query history efficiently.

## 2. Constraints

The schema uses database-enforced invariants:

- `account_number` is `UNIQUE`.
- `account_type` is restricted to Savings or Current.
- `balance >= 0`.
- `amount > 0`.
- Every transaction references an existing account through a foreign key.
- Deleting an account cascades to its transaction rows in this educational schema.

These constraints are important because application-level validation alone is not enough: the database remains the final consistency boundary.

## 3. Atomic deposit

A deposit consists logically of two writes:

```text
UPDATE account balance
        +
INSERT transaction record
```

Both should execute inside one database transaction:

```sql
BEGIN;

UPDATE accounts
SET balance = balance + :amount,
    updated_at = CURRENT_TIMESTAMP
WHERE account_id = :account_id;

INSERT INTO transactions (account_id, transaction_type, amount, balance_after)
SELECT account_id, 'Deposit', :amount, balance
FROM accounts
WHERE account_id = :account_id;

COMMIT;
```

If either operation fails, the service should roll back the entire transaction.

## 4. Atomic and concurrency-safe withdrawal

The important race condition is:

```text
Initial balance = 1000

Request A reads 1000 → wants 800
Request B reads 1000 → wants 700

Both pass a separate application-level check
```

This could incorrectly allow spending more than the available balance.

Instead, make the balance condition part of the database write:

```sql
BEGIN;

UPDATE accounts
SET balance = balance - :amount,
    updated_at = CURRENT_TIMESTAMP
WHERE account_id = :account_id
  AND balance >= :amount;
```

The service checks the affected-row count. If it is zero, the account does not exist or the balance is insufficient, so the transaction is rolled back.

If one row is updated, insert the transaction using the new balance:

```sql
INSERT INTO transactions (account_id, transaction_type, amount, balance_after)
SELECT account_id, 'Withdrawal', :amount, balance
FROM accounts
WHERE account_id = :account_id;

COMMIT;
```

The database row update provides the serialization point for concurrent withdrawals on the same account.

## 5. Isolation

For the normal debit/credit path, an atomic conditional update plus a database transaction is preferable to performing a read/check/update sequence entirely in application code.

For workflows that require stricter guarantees, the service can use an appropriate isolation level such as `SERIALIZABLE`, while handling serialization failures with bounded retries.

Isolation should be selected based on the invariant and workload rather than blindly using the strongest level everywhere.

## 6. Indexing

Important indexes include:

```sql
CREATE UNIQUE INDEX ... ON accounts(account_number);
CREATE INDEX ... ON transactions(account_id, created_at DESC);
```

This supports:

- fast account lookup by account number;
- efficient retrieval of an account's newest transactions;
- scalable transaction-history queries.

## 7. ACID properties in the design

### Atomicity

Balance update and transaction insertion commit together or roll back together.

### Consistency

Primary keys, foreign keys, unique constraints and check constraints protect database invariants.

### Isolation

Concurrent operations are controlled by database locking/isolation semantics rather than unsynchronized application-level reads and writes.

### Durability

After a successful commit, the database's durability mechanisms ensure committed data survives normal process failure.

## 8. Current implementation vs scalable target

| Concern | Current project | DBMS target |
|---|---|---|
| Storage | JSON file | Relational database |
| Account lookup | `unordered_map` | Indexed `account_number` |
| Transaction history | `vector<Transaction>` | `transactions` table |
| Atomicity | Not fully transactional | DB transaction / COMMIT / ROLLBACK |
| Concurrency | Single-process | DB locking + isolation |
| Constraints | Mostly application-level | DB-enforced + application validation |
| Money | `double` | `NUMERIC` / integer minor units |
| Persistence cost | Rewrite JSON dataset | Update affected rows |

The SQL in `sql/` is the concrete relational design for this next stage. The existing C++ JSON implementation remains useful as the local/demo backend.
