#pragma once
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cctype>
#include <string>
#include <unordered_map>
#include <ctime>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

typedef enum Type_ { SAVINGS = 0, CURRENT = 1 } Type;
extern const std::unordered_map<Type, std::string> accountTypeMap;

class User {
private:
    std::string account_number;
    std::string user_name;
    double account_balance;
    Type account_type;

    static int counter;
    static std::string getCurrentDate();

public:
    User();
    ~User();

    void createAccount();
    void displayAccount() const;
    void modifyAccount();
    void deleteAccount();
    void deposit(double amount);
    bool withdraw(double amount);

    std::string getAccountNumber() const;

    json toJson() const;
    static void saveToJson();
    static std::vector<User> loadFromJson();

    static void exportToCSV(const std::vector<User>& users);

    void setUserName(const std::string& name);
    void setAccountType(Type type);
    void setBalance(double balance);
    double getBalance() const;
};
