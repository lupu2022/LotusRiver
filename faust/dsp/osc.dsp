import("stdfaust.lib");

freq = hslider("freq", 440, 20, 20000, 0.1);

process = os.osc(freq);
