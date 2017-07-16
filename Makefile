TARGET  = aixlog_test
SHELL = /bin/bash

CXX      = /usr/bin/g++
CXXFLAGS = -Wall -O3 -std=c++11 -Iinclude

OBJ = aixlog_test.o
BIN = aixlog_test

all:	$(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)
	strip $(BIN)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN) $(OBJ) *~

