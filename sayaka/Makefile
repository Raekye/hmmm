CC = g++
CFLAGS = -g -std=c++11 -fexceptions
EXECUTABLE = sayaka
SRC_DIR = src
BIN_DIR = bin
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
SOURCES := $(filter-out $(SRC_DIR)/playground.cpp, $(SOURCES))

$(SRC_DIR)/lexer.cpp: $(SRC_DIR)/lexer.l
	cd $(SRC_DIR) && flex --outfile=lexer.cpp --header-file=lexer.h lexer.l

$(SRC_DIR)/parser.cpp: $(SRC_DIR)/parser.y
	cd $(SRC_DIR) && bison --defines=parser.h --output=parser.cpp --verbose --report-file=parser.log parser.y

build: $(SOURCES)
	$(CC) $(SOURCES) -o $(BIN_DIR)/$(EXECUTABLE) `llvm-config --libs core jit native --cxxflags --ldflags` $(CFLAGS)

prepare:
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)/*
	cd $(SRC_DIR) && rm -f lexer.cpp lexer.h parser.cpp parser.h parser.log

playground:
	$(CC) $(SRC_DIR)/playground.cpp -o $(BIN_DIR)/playground.o

all: prepare clean build

debug: all
	gdb $(BIN_DIR)/$(EXECUTABLE)
