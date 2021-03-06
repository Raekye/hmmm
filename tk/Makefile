CXX = g++
LD = g++
CXXFLAGS = -g -c -std=c++11 -Werror -Wall -Wextra -Wno-unknown-pragmas -fno-exceptions
LDFLAGS =
LDLIBS =

EXECUTABLE_NAME = tk

SRC_DIR = src
BIN_DIR = bin

EXECUTABLE = $(BIN_DIR)/$(EXECUTABLE_NAME)

SRCS = $(sort $(wildcard $(SRC_DIR)/*.cpp))
OBJS = $(addsuffix .o,$(subst $(SRC_DIR),$(BIN_DIR),$(SRCS)))
DEPS = $(sort $(wildcard $(BIN_DIR)/*.d))

.PHONY: all prepare build clean valgrind

all: prepare build

prepare: $(BIN_DIR)

build: $(EXECUTABLE)

clean:
	rm -rf $(BIN_DIR)

valgrind: all
	valgrind --leak-check=yes $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%
	$(CXX) $(CXXFLAGS) -MM $< -MT $@ -MF $(@:.o=.d)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_DIR):
	mkdir -p $@

include $(DEPS)
