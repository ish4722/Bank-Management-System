#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "user.h"

class Bank {
private:
    std::unordered_map<std::string, User> accounts;

public:
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
