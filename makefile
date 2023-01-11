GXX=/usr/bin/g++
GXXFLAGS= -O3 -std=c++17 -Wall -fmessage-length=0 -fopenmp
# GXXSANFLAG= -fsanitize=address
# GXXSANFLAG= -fsanitize=thread

.PHONY: mcts

all: mcts
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -o nogo build/mcts.o build/node.o build/state.o build/mc_rave.o build/ucb1.o  nogo.cpp

mcts:
	+make -C mcts

clean:
	rm build/**
	rm nogo
