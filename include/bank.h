#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include "database.h"
#include "user.h"

class Bank {
private:
    std::unordered_map<std::string, User> accounts;
    Database database;

    bool beginWriteTransaction();
    bool commitTransaction();
    void rollbackTransaction();
    bool reloadFromDatabase();

public:
    explicit Bank(const std::string& databasePath = "data/bank.db");

    bool load();
    bool save() const;

    bool createAccount(const std::string& name, Type type, std::string& accountNumber);
    User* findAccount(const std::string& accountNumber);
    const User* findAccount(const std::string& accountNumber) const;

    bool deposit(const std::string& accountNumber, double amount);
    bool withdraw(const std::string& accountNumber, double amount);
    bool modifyAccount(const std::string& accountNumber, const std::string& name, Type type);
    bool deleteAccount(const std::string& accountNumber);

    bool exportToCSV(const std::string& path = "data/accounts.csv") const;
    std::size_t size() const;
};
