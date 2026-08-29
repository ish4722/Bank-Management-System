#include "user.h"

#include <ctime>
#include <stdexcept>
#include <utility>

using namespace std;

const unordered_map<Type, string> accountTypeMap = {
    {Type::SAVINGS, "Savings"},
    {Type::CURRENT, "Current"}
};

namespace {
string currentTimestamp() {
    const time_t now = time(nullptr);
    tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
    return buffer;
}

string transactionTypeToString(TransactionType type) {
    return type == TransactionType::DEPOSIT ? "Deposit" : "Withdrawal";
}

TransactionType transactionTypeFromString(const string& type) {
    if (type == "Deposit") return TransactionType::DEPOSIT;
    if (type == "Withdrawal") return TransactionType::WITHDRAWAL;
    throw invalid_argument("Unknown transaction type: " + type);
}
}

json Transaction::toJson() const {
    return json{
        {"type", transactionTypeToString(type)},
        {"amount", amount},
        {"balance_after", balanceAfter},
        {"timestamp", timestamp}
    };
}

Transaction Transaction::fromJson(const json& data) {
    return Transaction{
        transactionTypeFromString(data.at("type").get<string>()),
        data.at("amount").get<double>(),
        data.at("balance_after").get<double>(),
        data.at("timestamp").get<string>()
    };
}

User::User(string accountNumber, string name, double balance, Type type)
    : account_number(std::move(accountNumber)),
      user_name(std::move(name)),
      account_balance(balance),
      account_type(type) {}

const string& User::getAccountNumber() const { return account_number; }
const string& User::getUserName() const { return user_name; }
double User::getBalance() const { return account_balance; }
Type User::getAccountType() const { return account_type; }
const vector<Transaction>& User::getTransactions() const { return transactions; }

void User::setUserName(const string& name) {
    if (name.empty()) throw invalid_argument("Name cannot be empty.");
    user_name = name;
}

void User::setAccountType(Type type) { account_type = type; }

void User::setBalance(double balance) {
    if (balance < 0.0) throw invalid_argument("Balance cannot be negative.");
    account_balance = balance;
}

bool User::deposit(double amount) {
    if (amount <= 0.0) return false;
    account_balance += amount;
    transactions.push_back({
        TransactionType::DEPOSIT,
        amount,
        account_balance,
        currentTimestamp()
    });
    return true;
}

bool User::withdraw(double amount) {
    if (amount <= 0.0 || amount > account_balance) return false;
    account_balance -= amount;
    transactions.push_back({
        TransactionType::WITHDRAWAL,
        amount,
        account_balance,
        currentTimestamp()
    });
    return true;
}

json User::toJson() const {
    json transactionArray = json::array();
    for (const auto& transaction : transactions) {
        transactionArray.push_back(transaction.toJson());
    }

    return json{
        {"account_number", account_number},
        {"user_name", user_name},
        {"account_balance", account_balance},
        {"account_type", accountTypeMap.at(account_type)},
        {"transactions", transactionArray}
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

    User user(
        data.at("account_number").get<string>(),
        data.at("user_name").get<string>(),
        data.at("account_balance").get<double>(),
        type
    );

    if (data.contains("transactions") && data["transactions"].is_array()) {
        for (const auto& item : data["transactions"]) {
            user.transactions.push_back(Transaction::fromJson(item));
        }
    }

    return user;
}

void User::displayAccount() const {
    cout << "Account Number: " << account_number << '\n';
    cout << "Name: " << user_name << '\n';
    cout << "Balance: $" << fixed << setprecision(2) << account_balance << '\n';
    cout << "Type: " << accountTypeMap.at(account_type) << '\n';
}

void User::displayTransactions() const {
    if (transactions.empty()) {
        cout << "No transactions found for this account.\n";
        return;
    }

    cout << "\n===== TRANSACTION HISTORY =====\n";
    cout << left << setw(22) << "Timestamp"
         << setw(15) << "Type"
         << setw(15) << "Amount"
         << "Balance After\n";
    cout << string(67, '-') << '\n';

    for (const auto& transaction : transactions) {
        cout << left << setw(22) << transaction.timestamp
             << setw(15) << transactionTypeToString(transaction.type)
             << setw(15) << fixed << setprecision(2) << transaction.amount
             << fixed << setprecision(2) << transaction.balanceAfter << '\n';
    }
}
