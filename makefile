all:
	#g++ -std=c++11 -O3 -g -Wall -fmessage-length=0 -o testcase test.cpp
	g++ -std=c++11 -O3 -g -Wall -fmessage-length=0 -o nogo nogo.cpp
clean:
	rm nogo