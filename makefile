GXX=/usr/bin/g++
GXXFLAGS= -g -std=c++11 -Wall -fmessage-length=0
GXXSANFLAG= -fsanitize=address

all:
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -o nogo mcts.cpp nogo.cpp
clean:
	rm nogo
