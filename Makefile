# Makefile for ID3

id3.out: ID3.h ID3.cpp main.cpp
	g++ -std=c++11 -O2 main.cpp ID3.cpp -o id3.out

.PHONY: clean

clean:
	rm id3.out
