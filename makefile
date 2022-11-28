GXX=/usr/bin/g++
GXXFLAGS= -O3 -std=c++11 -Wall -fmessage-length=0 -fopenmp
# GXXSANFLAG= -fsanitize=address

.PHONY: mcts

all: mcts
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -o nogo build/mcts.o build/node.o build/state.o build/selector.o  nogo.cpp

mcts:
	make -C mcts

clean:
	rm build/**
	rm nogo
