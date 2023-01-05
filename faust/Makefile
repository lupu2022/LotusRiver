define dsp2hpp
    faust -light -o auto/$(1)~.hpp -ns dsp -cn $(2) dsp/$(1).dsp 
endef
  
all:
	$(call dsp2hpp,sawtooth,OscSawtooth)
	$(call dsp2hpp,noise,OscNoise)
	$(call dsp2hpp,osc,OscSine)

clean:
	rm -rf auto/*.hpp
