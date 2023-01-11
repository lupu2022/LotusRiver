.PHONY: all

FLAGS = -std=c++17 -Wall -Wno-maybe-uninitialized -Wno-delete-non-virtual-dtor -fopenmp -O3 -D__LINUX_ALSA__
INC = -I. -I./eigen3

LINK = -lasound -lsndfile -lpthread -lm

all: synth 

io_rtaudio.o: io/RtAudio.cpp io/RtAudio.h
	g++ $(FLAGS) -c -o $@ io/RtAudio.cpp $(INC) 

io_rtmidi.o: io/RtMidi.cpp io/RtMidi.h
	g++ $(FLAGS) -c -o $@ io/RtMidi.cpp $(INC) 

nn_wavenet.o: lr.hpp nn/wavenet.hpp nn/wavenet.cpp
	g++ $(FLAGS) -c -o $@ nn/wavenet.cpp $(INC) 

faust_osc.o: lr.hpp faust/osc.hpp faust/osc.cpp
	g++ $(FLAGS) -c -o $@ faust/osc.cpp $(INC) 

faust_reverb.o: lr.hpp faust/reverb.hpp faust/reverb.cpp
	g++ $(FLAGS) -c -o $@ faust/reverb.cpp $(INC) 

lr.o: lr.hpp lr.cpp
	g++ $(FLAGS) -c -o $@ lr.cpp $(INC) 

synth: synth.cpp lr.hpp io/io_impl.hpp nn/nn_impl.hpp faust/faust_impl.hpp \
	lr.o \
	io_rtaudio.o \
	io_rtmidi.o \
	nn_wavenet.o \
	faust_osc.o \
	faust_reverb.o
	g++ $(FLAGS) -c -o synth.o synth.cpp $(INC)
	g++ $(FLAGS) -o $@ synth.o lr.o io_rtaudio.o nn_wavenet.o faust_osc.o faust_reverb.o $(LINK) 

clean:
	rm -f synth
	rm -f *.o
