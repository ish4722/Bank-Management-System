CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include
LDLIBS = -lsqlite3 -pthread

MAIN_SRC = main.cpp
USER_SRC = src/user.cpp
BANK_SRC = src/bank.cpp
DATABASE_SRC = src/database.cpp
USER_TEST_SRC = tests/user_test.cpp
BANK_TEST_SRC = tests/bank_test.cpp

OBJS = main.o user.o bank.o database.o
TARGET = bank
USER_TEST_TARGET = user_tests
BANK_TEST_TARGET = bank_tests

all: $(TARGET)

main.o: $(MAIN_SRC) include/bank.h include/user.h include/database.h
	$(CXX) $(CXXFLAGS) -c $(MAIN_SRC) -o main.o

user.o: $(USER_SRC) include/user.h include/json.hpp
	$(CXX) $(CXXFLAGS) -c $(USER_SRC) -o user.o

bank.o: $(BANK_SRC) include/bank.h include/user.h include/database.h
	$(CXX) $(CXXFLAGS) -c $(BANK_SRC) -o bank.o

database.o: $(DATABASE_SRC) include/database.h
	$(CXX) $(CXXFLAGS) -c $(DATABASE_SRC) -o database.o

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDLIBS)

$(USER_TEST_TARGET): $(USER_TEST_SRC) $(USER_SRC) include/user.h include/json.hpp tests/catch.hpp
	$(CXX) $(CXXFLAGS) $(USER_TEST_SRC) $(USER_SRC) -o $(USER_TEST_TARGET) $(LDLIBS)

$(BANK_TEST_TARGET): $(BANK_TEST_SRC) $(BANK_SRC) $(USER_SRC) $(DATABASE_SRC) include/bank.h include/user.h include/database.h include/json.hpp tests/catch.hpp
	$(CXX) $(CXXFLAGS) $(BANK_TEST_SRC) $(BANK_SRC) $(USER_SRC) $(DATABASE_SRC) -o $(BANK_TEST_TARGET) $(LDLIBS)

test: $(USER_TEST_TARGET) $(BANK_TEST_TARGET)
	./$(USER_TEST_TARGET)
	./$(BANK_TEST_TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(USER_TEST_TARGET) $(BANK_TEST_TARGET) data/*.db data/*.db-wal data/*.db-shm data/*.json data/*.csv

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run test
