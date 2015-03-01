CXX = g++
LD = g++
CXXFLAGS = -g -std=c++11 -c
LDFLAGS = -g
LDLIBS =

SRC_DIR = src
BIN_DIR = bin

EXECUTABLE = $(BIN_DIR)/siyu

SRCS = $(sort $(wildcard $(SRC_DIR)/*.cpp))
OBJS = $(addsuffix .o,$(subst $(SRC_DIR),$(BIN_DIR),$(SRCS)))
DEPS = $(addsuffix .d,$(subst $(SRC_DIR),$(BIN_DIR),$(SRCS)))

.PHONY: all build clean

all: build

build: $(EXECUTABLE)

clean:
	rm -rf $(BIN_DIR)

$(EXECUTABLE): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_DIR):
	mkdir -p $@

$(BIN_DIR)/%.d: $(SRC_DIR)/% $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -MM $< -MT '$(@:.d=.o)' -MF $@

include $(DEPS)
