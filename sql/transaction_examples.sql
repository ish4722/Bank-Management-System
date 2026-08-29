-- Transactional deposit
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


-- Transactional withdrawal with concurrency-safe balance check
BEGIN;

UPDATE accounts
SET balance = balance - :amount,
    updated_at = CURRENT_TIMESTAMP
WHERE account_id = :account_id
  AND balance >= :amount;

-- Application checks that exactly one row was updated.
-- If zero rows were updated, the account does not exist or funds are insufficient:
-- ROLLBACK;

INSERT INTO transactions (account_id, transaction_type, amount, balance_after)
SELECT account_id, 'Withdrawal', :amount, balance
FROM accounts
WHERE account_id = :account_id;

COMMIT;


-- Serializable isolation can be selected for especially strict workflows:
-- SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
