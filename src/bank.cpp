#include "bank.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>

using namespace std;

namespace {
string makeAccountNumber(const unordered_map<string, User>& accounts) {
    static mt19937_64 rng(random_device{}());
    uniform_int_distribution<long long> dist(1000000000LL, 9999999999LL);

    string number;
    do {
        number = to_string(dist(rng));
    } while (accounts.find(number) != accounts.end());
    return number;
}
}

bool Bank::load() {
    accounts.clear();
    ifstream inFile("data/accounts.json");
    if (!inFile) {
        return true; // First run: no data file yet.
    }

    try {
        json data;
        inFile >> data;
        if (!data.is_array()) {
            return false;
        }

        for (const auto& item : data) {
            User user = User::fromJson(item);
            accounts.emplace(user.getAccountNumber(), std::move(user));
        }
        return true;
    } catch (const exception&) {
        accounts.clear();
        return false;
    }
}

bool Bank::save() const {
    json data = json::array();
    for (const auto& [accountNumber, user] : accounts) {
        data.push_back(user.toJson());
    }

    ofstream outFile("data/accounts.json");
    if (!outFile) {
        return false;
    }

    outFile << data.dump(4);
    return static_cast<bool>(outFile);
}

bool Bank::createAccount(const string& name, Type type, string& accountNumber) {
    if (name.empty()) {
        return false;
    }

    accountNumber = makeAccountNumber(accounts);
    accounts.emplace(accountNumber, User(accountNumber, name, 0.0, type));
    return save();
}

User* Bank::findAccount(const string& accountNumber) {
    auto it = accounts.find(accountNumber);
    return it == accounts.end() ? nullptr : &it->second;
}

const User* Bank::findAccount(const string& accountNumber) const {
    auto it = accounts.find(accountNumber);
    return it == accounts.end() ? nullptr : &it->second;
}

bool Bank::deposit(const string& accountNumber, double amount) {
    User* user = findAccount(accountNumber);
    if (!user || !user->deposit(amount)) {
        return false;
    }
    return save();
}

bool Bank::withdraw(const string& accountNumber, double amount) {
    User* user = findAccount(accountNumber);
    if (!user || !user->withdraw(amount)) {
        return false;
    }
    return save();
}

bool Bank::modifyAccount(const string& accountNumber, const string& name, Type type) {
    User* user = findAccount(accountNumber);
    if (!user || name.empty()) {
        return false;
    }

    user->setUserName(name);
    user->setAccountType(type);
    return save();
}

bool Bank::deleteAccount(const string& accountNumber) {
    auto it = accounts.find(accountNumber);
    if (it == accounts.end()) {
        return false;
    }

    accounts.erase(it);
    return save();
}

bool Bank::exportToCSV(const string& path) const {
    ofstream outFile(path);
    if (!outFile) {
        return false;
    }

    outFile << "Account Number,Name,Balance,Type\n";
    for (const auto& [accountNumber, user] : accounts) {
        outFile << accountNumber << ','
                << user.getUserName() << ','
                << fixed << setprecision(2) << user.getBalance() << ','
                << accountTypeMap.at(user.getAccountType()) << '\n';
    }

    return static_cast<bool>(outFile);
}

size_t Bank::size() const {
    return accounts.size();
}
