.PHONY: all

FLAGS = -std=c++17 -Wall -Wno-maybe-uninitialized -Wno-delete-non-virtual-dtor -fopenmp -O3
INC = -I. -I./eigen3

LINK = -lsndfile -lpthread -lm

all: synth 

nn_wavenet.o: lr.hpp nn/wavenet.hpp nn/wavenet.cpp
	g++ $(FLAGS) -c -o $@ nn/wavenet.cpp $(INC) 

faust_osc.o: lr.hpp faust/osc.hpp faust/osc.cpp
	g++ $(FLAGS) -c -o $@ faust/osc.cpp $(INC) 

lr.o: lr.hpp lr.cpp
	g++ $(FLAGS) -c -o $@ lr.cpp $(INC) 

synth: synth.cpp lr.hpp io/io_impl.hpp nn/nn_impl.hpp faust/faust_impl.hpp \
	lr.o \
	nn_wavenet.o \
	faust_osc.o 
	g++ $(FLAGS) -c -o synth.o synth.cpp $(INC)
	g++ $(FLAGS) -o $@ synth.o lr.o nn_wavenet.o faust_osc.o $(LINK) 

clean:
	rm -f synth
	rm -f *.o
