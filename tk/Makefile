CXX = g++
LD = g++
CXXFLAGS = -g -c -std=c++11 -Werror -Wall -Wextra -Wno-unknown-pragmas -fno-exceptions
LDFLAGS =
LDLIBS =

SRC_DIR = src
BIN_DIR = bin

EXECUTABLE = $(BIN_DIR)/tk

SRCS = $(sort $(wildcard $(SRC_DIR)/*.cpp))
OBJS = $(addsuffix .o,$(subst $(SRC_DIR),$(BIN_DIR),$(SRCS)))
DEPS = $(sort $(wildcard $(BIN_DIR)/*.d))

.PHONY: all prepare build clean

all: prepare build

prepare: $(BIN_DIR)

build: $(EXECUTABLE)

clean:
	rm -rf $(BIN_DIR)

$(EXECUTABLE): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%
	$(CXX) $(CXXFLAGS) -MM $< -MT $@ -MF $(@:.o=.d)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_DIR):
	mkdir -p $@

include $(DEPS)
