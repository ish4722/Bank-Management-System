CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include

MAIN_SRC = main.cpp
USER_SRC = src/user.cpp
BANK_SRC = src/bank.cpp
TEST_SRC = tests/user_test.cpp

OBJS = main.o user.o bank.o
TARGET = bank
TEST_TARGET = user_tests

all: $(TARGET)

main.o: $(MAIN_SRC) include/bank.h include/user.h
	$(CXX) $(CXXFLAGS) -c $(MAIN_SRC) -o main.o

user.o: $(USER_SRC) include/user.h include/json.hpp
	$(CXX) $(CXXFLAGS) -c $(USER_SRC) -o user.o

bank.o: $(BANK_SRC) include/bank.h include/user.h include/json.hpp
	$(CXX) $(CXXFLAGS) -c $(BANK_SRC) -o bank.o

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

$(TEST_TARGET): $(TEST_SRC) $(USER_SRC) include/user.h include/json.hpp tests/catch.hpp
	$(CXX) $(CXXFLAGS) $(TEST_SRC) $(USER_SRC) -o $(TEST_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(TEST_TARGET) data/*.json data/*.csv

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run test
