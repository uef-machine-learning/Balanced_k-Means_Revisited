all: bkmeans

bkmeans: *cpp
	gcc -c contrib/argtable/argtable3.c
	g++ -O3 -std=c++11   sources.cpp argtable3.o -o bkmeans
	
bkmeans_debug: *cpp
	gcc -c contrib/argtable/argtable3.c
	g++ -O0 -g -std=c++11   sources.cpp argtable3.o -o bkmeans_debug


