GXX=/usr/bin/g++
GXXFLAGS= -g -std=c++11 -Wall -fmessage-length=0
GXXSANFLAG= -fsanitize=address

BUILD_DIR= '../build'

all: node state selector mcts

node: node.cpp mcts.h
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -c node.cpp -o $(BUILD_DIR)/node.o

state: state.cpp mcts.h
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -c state.cpp -o $(BUILD_DIR)/state.o

selector: selector/ucb1.cpp mcts.h
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -c selector/ucb1.cpp -o $(BUILD_DIR)/selector.o

mcts: mcts.cpp mcts.h
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -c mcts.cpp -o $(BUILD_DIR)/mcts.o