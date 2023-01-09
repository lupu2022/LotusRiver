import("stdfaust.lib");

freq = nentry("freq", 440, 20, 20000, 0.1);

process = os.triangle(freq);
