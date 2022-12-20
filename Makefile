.PHONY: all

FLAGS = -std=c++17 -Wall -fopenmp -g
INC = -I. -I./eigen3

LINK = -lsndfile -lpthread -lm

all: synth 

synth: synth.cpp lr.hpp io/io_impl.hpp 
	g++ $(FLAGS) -c -o synth.o synth.cpp $(INC)
	g++ $(FLAGS) -o $@ synth.o $(LINK) 

clean:
	rm -f synth 
	rm -f *.o
