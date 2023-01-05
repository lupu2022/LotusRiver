#ifndef __DSP_hpp__
#define __DSP_hpp__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

namespace faust {

class dsp {};
class Meta {
public:
    virtual void declare(const char* key, const char* value) {}
};

class UI {
public:
    virtual void openHorizontalBox(const char* name) {}
    virtual void openVerticalBox(const char* name) {}
    virtual void declare(FAUSTFLOAT* entry, const char* key, const char* value) {}
    virtual void addNumEntry(const char* name, FAUSTFLOAT* entry, FAUSTFLOAT defvalue, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) {
        set_parameter(name, entry);
    }
    virtual void addHorizontalSlider(const char* name, FAUSTFLOAT* entry, FAUSTFLOAT defvalue, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) {
        set_parameter(name, entry);
    }
    virtual void addButton(const char* name, FAUSTFLOAT* entry) {
        set_parameter(name, entry);
    }

    virtual void closeBox() {}


    FAUSTFLOAT* gain;
    FAUSTFLOAT* gate;
    FAUSTFLOAT* freq;
private:
    void set_parameter(const char* name, FAUSTFLOAT* target) {
        if ( strcmp(name, "gain") == 0) {
            gain = target;
        } else if ( strcmp(name, "gate") == 0 ) {
            gate = target;
        } else if ( strcmp(name, "freq") == 0 ) {
            freq = target;
        }
    }
};

}
#endif
