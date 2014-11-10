CC = g++
CFLAGS = -c -g -std=c++11 -fexceptions
LDFLAGS = -g -rdynamic
LLVM_COMPILER_FLAGS = $(shell llvm-config --cxxflags)
LLVM_LINKER_FLAGS = $(shell llvm-config --ldflags --libs core jit native)
EXECUTABLE = sayaka
SRC_DIR = src
BIN_DIR = bin
SOURCES = $(filter-out $(SRC_DIR)/playground.cpp, $(wildcard $(SRC_DIR)/*.cpp))
LEXER_SOURCE = $(SRC_DIR)/lexer.cpp
PARSER_SOURCE = $(SRC_DIR)/parser.cpp
OBJS = $(subst $(SRC_DIR),$(BIN_DIR),$(subst .cpp,.o,$(SOURCES)))

all: $(BIN_DIR) $(PARSER_SOURCE) $(LEXER_SOURCE) $(BIN_DIR)/$(EXECUTABLE)

$(BIN_DIR)/$(EXECUTABLE): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LLVM_LINKER_FLAGS) $(LDFLAGS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(LLVM_COMPILER_FLAGS) $(CFLAGS) $< -o $@

$(LEXER_SOURCE): $(SRC_DIR)/lexer.l
	flex --outfile=$@ --header-file=$(SRC_DIR)/lexer.h $(SRC_DIR)/lexer.l

$(PARSER_SOURCE): $(SRC_DIR)/parser.y
	bison --defines=$(SRC_DIR)/parser.h --output=$@ --verbose --report-file=$(BIN_DIR)/parser.log $(SRC_DIR)/parser.y

debug: all
	gdb $(BIN_DIR)/$(EXECUTABLE)

valgrind: all
	valgrind --leak-check=yes $(BIN_DIR)/$(EXECUTABLE)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)
	rm -f $(LEXER_SOURCE) $(SRC_DIR)/lexer.h $(PARSER_SOURCE) $(SRC_DIR)/parser.h

playground: $(BIN_DIR)
	$(CC) $(SRC_DIR)/playground.cpp -o $(BIN_DIR)/playground.o $(shell llvm-config --libs core jit native --cxxflags --ldflags)
