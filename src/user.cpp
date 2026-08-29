#include "user.h"

#include <stdexcept>

using namespace std;

const unordered_map<Type, string> accountTypeMap = {
    {Type::SAVINGS, "Savings"},
    {Type::CURRENT, "Current"}
};

User::User(string accountNumber, string name, double balance, Type type)
    : account_number(std::move(accountNumber)),
      user_name(std::move(name)),
      account_balance(balance),
      account_type(type) {}

const string& User::getAccountNumber() const {
    return account_number;
}

const string& User::getUserName() const {
    return user_name;
}

double User::getBalance() const {
    return account_balance;
}

Type User::getAccountType() const {
    return account_type;
}

void User::setUserName(const string& name) {
    if (name.empty()) {
        throw invalid_argument("Name cannot be empty.");
    }
    user_name = name;
}

void User::setAccountType(Type type) {
    account_type = type;
}

void User::setBalance(double balance) {
    if (balance < 0.0) {
        throw invalid_argument("Balance cannot be negative.");
    }
    account_balance = balance;
}

bool User::deposit(double amount) {
    if (amount <= 0.0) {
        return false;
    }
    account_balance += amount;
    return true;
}

bool User::withdraw(double amount) {
    if (amount <= 0.0 || amount > account_balance) {
        return false;
    }
    account_balance -= amount;
    return true;
}

json User::toJson() const {
    return json{
        {"account_number", account_number},
        {"user_name", user_name},
        {"account_balance", account_balance},
        {"account_type", accountTypeMap.at(account_type)}
    };
}

User User::fromJson(const json& data) {
    Type type = Type::SAVINGS;
    const string typeName = data.at("account_type").get<string>();

    for (const auto& [candidateType, candidateName] : accountTypeMap) {
        if (candidateName == typeName) {
            type = candidateType;
            break;
        }
    }

    return User(
        data.at("account_number").get<string>(),
        data.at("user_name").get<string>(),
        data.at("account_balance").get<double>(),
        type
    );
}

void User::displayAccount() const {
    cout << "Account Number: " << account_number << '\n';
    cout << "Name: " << user_name << '\n';
    cout << "Balance: $" << fixed << setprecision(2) << account_balance << '\n';
    cout << "Type: " << accountTypeMap.at(account_type) << '\n';
}
