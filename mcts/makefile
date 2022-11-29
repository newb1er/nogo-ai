GXX=/usr/bin/g++
GXXFLAGS= -O3 -std=c++11 -Wall -fmessage-length=0 -fopenmp
# GXXSANFLAG= -fsanitize=address

BUILD_DIR= ../build

.PHONY: selector

all: node state selector mcts
	mkdir -p $(BUILD_DIR)

node: node.cpp mcts.h
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -c node.cpp -o $(BUILD_DIR)/node.o

state: state.cpp mcts.h
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -c state.cpp -o $(BUILD_DIR)/state.o

selector: selector/ucb1.cpp mcts.h
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -c selector/ucb1.cpp -o $(BUILD_DIR)/selector.o

mcts: mcts.cpp mcts.h
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -c mcts.cpp -o $(BUILD_DIR)/mcts.o