#include "bank.h"

#include <limits>

using namespace std;

namespace {
int readChoice() {
    int choice;
    while (!(cin >> choice)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Enter a number: ";
    }
    return choice;
}

Type readAccountType() {
    int type;
    while (true) {
        cout << "Enter Account Type (0 for Savings, 1 for Current): ";
        if (cin >> type && (type == 0 || type == 1)) {
            return type == 0 ? Type::SAVINGS : Type::CURRENT;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid choice! Please enter 0 or 1.\n";
    }
}

string readName() {
    string name;
    cout << "Enter Your Name: ";
    getline(cin >> ws, name);
    return name;
}

double readAmount(const string& prompt) {
    double amount;
    while (true) {
        cout << prompt;
        if (cin >> amount && amount > 0.0) return amount;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid amount. Please enter a positive number.\n";
    }
}

string readAccountNumber() {
    string number;
    cout << "Enter Account Number: ";
    cin >> number;
    return number;
}
}

int main() {
    Bank bank;
    if (!bank.load()) {
        cerr << "Error: Could not load account data.\n";
        return 1;
    }

    int choice;
    do {
        cout << "\n==== WELCOME TO BANK MANAGEMENT SYSTEM ====\n";
        cout << "1. Create Account\n";
        cout << "2. Display Account\n";
        cout << "3. Modify Account\n";
        cout << "4. Delete Account\n";
        cout << "5. Deposit Money\n";
        cout << "6. Withdraw Money\n";
        cout << "7. View Transaction History\n";
        cout << "8. Export Accounts to CSV\n";
        cout << "9. Exit\n";
        cout << "Enter your choice: ";
        choice = readChoice();

        switch (choice) {
            case 1: {
                const string name = readName();
                const Type type = readAccountType();
                string accountNumber;
                if (bank.createAccount(name, type, accountNumber))
                    cout << "Account " << accountNumber << " created successfully!\n";
                else
                    cout << "Could not create account.\n";
                break;
            }
            case 2: {
                const User* user = bank.findAccount(readAccountNumber());
                if (user) user->displayAccount();
                else cout << "Account Not Found!\n";
                break;
            }
            case 3: {
                const string accountNumber = readAccountNumber();
                const string name = readName();
                const Type type = readAccountType();
                if (bank.modifyAccount(accountNumber, name, type))
                    cout << "Account Details Updated Successfully!\n";
                else
                    cout << "Account Not Found or update failed!\n";
                break;
            }
            case 4: {
                if (bank.deleteAccount(readAccountNumber()))
                    cout << "Account deleted successfully.\n";
                else
                    cout << "Account Not Found!\n";
                break;
            }
            case 5: {
                const string accountNumber = readAccountNumber();
                const double amount = readAmount("Enter Amount to Deposit: ");
                if (bank.deposit(accountNumber, amount))
                    cout << "Deposit successful.\n";
                else
                    cout << "Account Not Found or deposit failed!\n";
                break;
            }
            case 6: {
                const string accountNumber = readAccountNumber();
                const double amount = readAmount("Enter Amount to Withdraw: ");
                if (bank.withdraw(accountNumber, amount))
                    cout << "Withdrawal successful.\n";
                else
                    cout << "Account Not Found, invalid amount, or insufficient balance!\n";
                break;
            }
            case 7: {
                const User* user = bank.findAccount(readAccountNumber());
                if (user) user->displayTransactions();
                else cout << "Account Not Found!\n";
                break;
            }
            case 8:
                cout << (bank.exportToCSV() ? "Accounts exported successfully to CSV.\n"
                                             : "Error exporting accounts to CSV!\n");
                break;
            case 9:
                cout << "Exiting... Have a Nice Day!\n";
                break;
            default:
                cout << "Invalid choice! Please try again.\n";
        }
    } while (choice != 9);

    return 0;
}
