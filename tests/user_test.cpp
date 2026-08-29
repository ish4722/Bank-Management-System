#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../include/user.h"

User createTestUser(const std::string& name = "Test User", Type type = Type::SAVINGS, double balance = 0.0) {
    User user;
    user.setUserName(name);
    user.setAccountType(type);
    user.setBalance(balance);
    return user;
}

TEST_CASE("Deposit increases balance correctly", "[deposit]") {
    User u = createTestUser();
    REQUIRE(u.deposit(100.0) == true);
    REQUIRE(u.getBalance() == Approx(100.0));
}

TEST_CASE("Deposit rejects non-positive amounts", "[deposit]") {
    User u = createTestUser("Invalid Deposit", Type::SAVINGS, 100.0);
    REQUIRE(u.deposit(0.0) == false);
    REQUIRE(u.deposit(-50.0) == false);
    REQUIRE(u.getBalance() == Approx(100.0));
}

TEST_CASE("Withdraw reduces balance if sufficient", "[withdraw]") {
    User u = createTestUser("Withdraw Test", Type::SAVINGS, 200.0);
    REQUIRE(u.withdraw(150.0) == true);
    REQUIRE(u.getBalance() == Approx(50.0));
}

TEST_CASE("Withdraw fails with insufficient balance", "[withdraw]") {
    User u = createTestUser("Low Funds", Type::CURRENT, 50.0);
    REQUIRE(u.withdraw(100.0) == false);
    REQUIRE(u.getBalance() == Approx(50.0));
}

TEST_CASE("Withdraw rejects non-positive amounts", "[withdraw]") {
    User u = createTestUser("Invalid Withdrawal", Type::CURRENT, 100.0);
    REQUIRE(u.withdraw(0.0) == false);
    REQUIRE(u.withdraw(-20.0) == false);
    REQUIRE(u.getBalance() == Approx(100.0));
}

TEST_CASE("Account type is correctly serialized", "[account_type]") {
    User u = createTestUser("Typed User", Type::CURRENT);
    json j = u.toJson();
    REQUIRE(j["account_type"] == "Current");
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
}
