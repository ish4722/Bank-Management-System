#include "catch.hpp"
#include "../include/bank.h"
#include <filesystem>
#include <string>
#include <thread>

namespace {
std::string dbPath(const std::string& name) { return "data/" + name + ".db"; }
void cleanup(const std::string& path) {
    std::filesystem::remove(path);
    std::filesystem::remove(path + "-wal");
    std::filesystem::remove(path + "-shm");
}
}

TEST_CASE("SQLite persistence keeps account and transaction history", "[database]") {
    const std::string path = dbPath("bank_persistence_test");
    cleanup(path);

    std::string accountNumber;
    {
        Bank bank(path);
        REQUIRE(bank.load());
        REQUIRE(bank.createAccount("SQLite User", Type::SAVINGS, accountNumber));
        REQUIRE(bank.deposit(accountNumber, 100.50));
        REQUIRE(bank.withdraw(accountNumber, 25.25));
    }

    {
        Bank bank(path);
        REQUIRE(bank.load());
        const User* user = bank.findAccount(accountNumber);
        REQUIRE(user != nullptr);
        REQUIRE(user->getBalance() == Approx(75.25));
        REQUIRE(user->getTransactions().size() == 2);
    }

    cleanup(path);
}

TEST_CASE("Concurrent withdrawals cannot overspend an account", "[concurrency]") {
    const std::string path = dbPath("bank_concurrency_test");
    cleanup(path);

    std::string accountNumber;
    {
        Bank setup(path);
        REQUIRE(setup.load());
        REQUIRE(setup.createAccount("Concurrent User", Type::SAVINGS, accountNumber));
        REQUIRE(setup.deposit(accountNumber, 100.0));
    }

    bool first = false;
    bool second = false;
    std::thread t1([&] { Bank bank(path); bank.load(); first = bank.withdraw(accountNumber, 100.0); });
    std::thread t2([&] { Bank bank(path); bank.load(); second = bank.withdraw(accountNumber, 100.0); });
    t1.join();
    t2.join();

    REQUIRE((first ? 1 : 0) + (second ? 1 : 0) == 1);

    Bank verify(path);
    REQUIRE(verify.load());
    const User* user = verify.findAccount(accountNumber);
    REQUIRE(user != nullptr);
    REQUIRE(user->getBalance() == Approx(0.0));
    REQUIRE(user->getTransactions().size() == 2);

    cleanup(path);
}

TEST_CASE("Failed withdrawal is atomic", "[acid]") {
    const std::string path = dbPath("bank_atomicity_test");
    cleanup(path);

    std::string accountNumber;
    Bank bank(path);
    REQUIRE(bank.load());
    REQUIRE(bank.createAccount("Atomic User", Type::SAVINGS, accountNumber));
    REQUIRE(bank.deposit(accountNumber, 50.0));
    REQUIRE_FALSE(bank.withdraw(accountNumber, 75.0));

    const User* user = bank.findAccount(accountNumber);
    REQUIRE(user != nullptr);
    REQUIRE(user->getBalance() == Approx(50.0));
    REQUIRE(user->getTransactions().size() == 1);

    cleanup(path);
}
