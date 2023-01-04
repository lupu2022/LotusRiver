.PHONY: all

FLAGS = -std=c++17 -Wall -Wno-maybe-uninitialized -fopenmp -O3
INC = -I. -I./eigen3

LINK = -lsndfile -lpthread -lm

all: synth 

wavenet.o: lr.hpp nn/wavenet.hpp nn/wavenet.cpp
	g++ $(FLAGS) -c -o $@ nn/wavenet.cpp $(INC) 

lr.o: lr.hpp lr.cpp
	g++ $(FLAGS) -c -o $@ lr.cpp $(INC) 

synth: synth.cpp lr.hpp io/io_impl.hpp nn/nn_impl.hpp lr.o wavenet.o 
	g++ $(FLAGS) -c -o synth.o synth.cpp $(INC)
	g++ $(FLAGS) -o $@ synth.o wavenet.o lr.o $(LINK) 

clean:
	rm -f synth 
	rm -f *.o
