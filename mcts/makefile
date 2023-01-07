GXX=/usr/bin/g++
GXXFLAGS= -O3 -std=c++17 -Wall -fmessage-length=0 -fopenmp
# GXXSANFLAG= -fsanitize=address
# GXXSANFLAG= -fsanitize=thread

BUILD_DIR= ../build

SELECTOR= mc_rave

all: node state rave ucb1 mcts
	mkdir -p $(BUILD_DIR)

node: node.cpp mcts.h
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -c node.cpp -o $(BUILD_DIR)/node.o

state: state.cpp mcts.h
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -c state.cpp -o $(BUILD_DIR)/state.o

ucb1: selector/ucb1.cpp mcts.h
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -c selector/ucb1.cpp -o $(BUILD_DIR)/ucb1.o

rave: selector/mc_rave.cpp mcts.h
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -c selector/mc_rave.cpp -o $(BUILD_DIR)/mc_rave.o

mcts: mcts.cpp mcts.h
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -c mcts.cpp -o $(BUILD_DIR)/mcts.o