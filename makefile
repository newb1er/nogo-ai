GXX=/usr/bin/g++
GXXFLAGS= -g -std=c++11 -Wall -fmessage-length=0
GXXSANFLAG= -fsanitize=address

.PHONY: mcts

all: mcts nogo
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -o nogo build/mcts.o build/node.o build/state.o build/selector.o  nogo.cpp

nogo: mcts
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -o nogo build/mcts.o build/node.o build/state.o build/selector.o nogo.cpp

mcts:
	make -C mcts

clean:
	rm build/**
	rm nogo
