#pragma once

#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

enum class Type { SAVINGS = 0, CURRENT = 1 };
enum class TransactionType { DEPOSIT, WITHDRAWAL };

struct Transaction {
    TransactionType type;
    double amount;
    double balanceAfter;
    std::string timestamp;

    json toJson() const;
    static Transaction fromJson(const json& data);
};

extern const std::unordered_map<Type, std::string> accountTypeMap;

class User {
private:
    std::string account_number;
    std::string user_name;
    double account_balance = 0.0;
    Type account_type = Type::SAVINGS;
    std::vector<Transaction> transactions;

public:
    User() = default;
    User(std::string accountNumber, std::string name, double balance, Type type);

    const std::string& getAccountNumber() const;
    const std::string& getUserName() const;
    double getBalance() const;
    Type getAccountType() const;
    const std::vector<Transaction>& getTransactions() const;

    void setUserName(const std::string& name);
    void setAccountType(Type type);
    void setBalance(double balance);

    bool deposit(double amount);
    bool withdraw(double amount);

    json toJson() const;
    static User fromJson(const json& data);

    void displayAccount() const;
    void displayTransactions() const;
};
