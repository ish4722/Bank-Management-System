#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../include/user.h"

User createTestUser(const std::string& name = "Test User", Type type = Type::SAVINGS, double balance = 0.0) {
    User user("1234567890", name, balance, type);
    return user;
}

TEST_CASE("Deposit increases balance and records transaction", "[deposit]") {
    User u = createTestUser();
    REQUIRE(u.deposit(100.0) == true);
    REQUIRE(u.getBalance() == Approx(100.0));
    REQUIRE(u.getTransactions().size() == 1);
    REQUIRE(u.getTransactions()[0].type == TransactionType::DEPOSIT);
    REQUIRE(u.getTransactions()[0].amount == Approx(100.0));
    REQUIRE(u.getTransactions()[0].balanceAfter == Approx(100.0));
}

TEST_CASE("Deposit rejects non-positive amounts without recording transaction", "[deposit]") {
    User u = createTestUser("Invalid Deposit", Type::SAVINGS, 100.0);
    REQUIRE(u.deposit(0.0) == false);
    REQUIRE(u.deposit(-50.0) == false);
    REQUIRE(u.getBalance() == Approx(100.0));
    REQUIRE(u.getTransactions().empty());
}

TEST_CASE("Withdraw reduces balance and records transaction", "[withdraw]") {
    User u = createTestUser("Withdraw Test", Type::SAVINGS, 200.0);
    REQUIRE(u.withdraw(150.0) == true);
    REQUIRE(u.getBalance() == Approx(50.0));
    REQUIRE(u.getTransactions().size() == 1);
    REQUIRE(u.getTransactions()[0].type == TransactionType::WITHDRAWAL);
    REQUIRE(u.getTransactions()[0].amount == Approx(150.0));
    REQUIRE(u.getTransactions()[0].balanceAfter == Approx(50.0));
}

TEST_CASE("Withdraw fails with insufficient balance and records nothing", "[withdraw]") {
    User u = createTestUser("Low Funds", Type::CURRENT, 50.0);
    REQUIRE(u.withdraw(100.0) == false);
    REQUIRE(u.getBalance() == Approx(50.0));
    REQUIRE(u.getTransactions().empty());
}

TEST_CASE("Withdraw rejects non-positive amounts", "[withdraw]") {
    User u = createTestUser("Invalid Withdrawal", Type::CURRENT, 100.0);
    REQUIRE(u.withdraw(0.0) == false);
    REQUIRE(u.withdraw(-20.0) == false);
    REQUIRE(u.getBalance() == Approx(100.0));
    REQUIRE(u.getTransactions().empty());
}

TEST_CASE("Account type is correctly serialized", "[account_type]") {
    User u = createTestUser("Typed User", Type::CURRENT);
    json j = u.toJson();
    REQUIRE(j["account_type"] == "Current");
}

TEST_CASE("Transaction history is serialized and restored", "[transactions][json]") {
    User original = createTestUser("History User", Type::SAVINGS, 100.0);
    REQUIRE(original.deposit(50.0));
    REQUIRE(original.withdraw(25.0));

    User restored = User::fromJson(original.toJson());

    REQUIRE(restored.getBalance() == Approx(125.0));
    REQUIRE(restored.getTransactions().size() == 2);
    REQUIRE(restored.getTransactions()[0].type == TransactionType::DEPOSIT);
    REQUIRE(restored.getTransactions()[0].amount == Approx(50.0));
    REQUIRE(restored.getTransactions()[0].balanceAfter == Approx(150.0));
    REQUIRE(restored.getTransactions()[1].type == TransactionType::WITHDRAWAL);
    REQUIRE(restored.getTransactions()[1].amount == Approx(25.0));
    REQUIRE(restored.getTransactions()[1].balanceAfter == Approx(125.0));
    REQUIRE(!restored.getTransactions()[0].timestamp.empty());
}

TEST_CASE("Account fields can be updated", "[setters]") {
    User u = createTestUser();
    u.setUserName("Updated User");
    u.setAccountType(Type::CURRENT);
    u.setBalance(500.0);

    REQUIRE(u.getUserName() == "Updated User");
    REQUIRE(u.getAccountType() == Type::CURRENT);
    REQUIRE(u.getBalance() == Approx(500.0));
}

TEST_CASE("Account fields round-trip through JSON", "[json]") {
    User original("1234567890", "JSON User", 1250.50, Type::SAVINGS);
    User restored = User::fromJson(original.toJson());

    REQUIRE(restored.getAccountNumber() == original.getAccountNumber());
    REQUIRE(restored.getUserName() == original.getUserName());
    REQUIRE(restored.getBalance() == Approx(original.getBalance()));
    REQUIRE(restored.getAccountType() == Type::SAVINGS);
    REQUIRE(restored.getTransactions().empty());
}
