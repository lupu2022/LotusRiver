import("stdfaust.lib");

freq = nentry("freq",440,20,20000,1);
gain = nentry("gain",1,0,1,0.01); 

process = os.sawtooth(freq) * gain;
