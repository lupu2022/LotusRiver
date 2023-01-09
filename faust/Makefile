define dsp2hpp
    faust -light -o auto/$(1)~.hpp -ns dsp -cn $(2) dsp/$(1).dsp 
endef
  
all:
	$(call dsp2hpp,sawtooth,OscSawtooth)
	$(call dsp2hpp,osc,OscSine)
	$(call dsp2hpp,square,OscSquare)
	$(call dsp2hpp,triangle,OscTriangle))
	$(call dsp2hpp,noise,NoiseWhite)

clean:
	rm -rf auto/*.hpp
