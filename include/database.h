#pragma once

#include <sqlite3.h>
#include <string>

class Database {
private:
    sqlite3* db_ = nullptr;

public:
    explicit Database(const std::string& path = "data/bank.db");
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    bool open();
    void close();
    bool initializeSchema();
    bool execute(const std::string& sql) const;
    sqlite3* handle() const { return db_; }
};
