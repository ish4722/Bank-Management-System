#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

enum class Type { SAVINGS = 0, CURRENT = 1 };

extern const std::unordered_map<Type, std::string> accountTypeMap;

class User {
private:
    std::string account_number;
    std::string user_name;
    double account_balance = 0.0;
    Type account_type = Type::SAVINGS;

public:
    User() = default;
    User(std::string accountNumber, std::string name, double balance, Type type);

    const std::string& getAccountNumber() const;
    const std::string& getUserName() const;
    double getBalance() const;
    Type getAccountType() const;

    void setUserName(const std::string& name);
    void setAccountType(Type type);
    void setBalance(double balance);

    bool deposit(double amount);
    bool withdraw(double amount);

    json toJson() const;
    static User fromJson(const json& data);

    void displayAccount() const;
};
