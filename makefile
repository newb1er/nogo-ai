GXX=/usr/bin/g++
GXXFLAGS= -g -std=c++11 -O3 -Wall -fmessage-length=0
GXXSANFLAG= -fsanitize=address

all:
	$(GXX) $(GXXFLAGS) $(GXXSANFLAG) -o nogo nogo.cpp
clean:
	rm nogo
