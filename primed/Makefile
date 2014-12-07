CXX = g++
LD = g++
CXXFLAGS = -g -std=c++11 -c
LDFLAGS = -g
LDLIBS =

SRC_DIR = src
BIN_DIR = bin
EXECUTABLE = primed

SOURCES = $(sort $(wildcard $(SRC_DIR)/*.cpp))
OBJS = $(subst $(SRC_DIR),$(BIN_DIR),$(subst .cpp,.o,$(SOURCES)))

all: prepare build

build: $(BIN_DIR)/$(EXECUTABLE)

$(BIN_DIR)/$(EXECUTABLE): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

prepare: $(BIN_DIR)

$(BIN_DIR):
	mkdir -p $@

clean:
	rm -rf $(BIN_DIR)
