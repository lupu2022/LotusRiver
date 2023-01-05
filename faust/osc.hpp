#ifndef _FAUST_OSC_HPP_
#define _FAUST_OSC_HPP_

#include "lr.hpp"
#include "faust/dsp.hpp"

namespace dsp {
    class OscSine;
    class OscNoise;
    class OscSawtooth;
}

namespace lr { namespace faust {

struct OscSineWord : public NativeWord {
    OscSineWord() { dsp = nullptr; }
    virtual ~OscSineWord();
    virtual void run(Stack& stack);

    NWORD_CREATOR_DEFINE_LR(OscSineWord)

private:
    dsp::OscSine* dsp;
    dsp::UI ui;
    Vec vec;
};

struct OscNoiseWord : public NativeWord {
    OscNoiseWord() { dsp = nullptr; }
    virtual ~OscNoiseWord();
    virtual void run(Stack& stack);

    NWORD_CREATOR_DEFINE_LR(OscNoiseWord)
private:
    dsp::OscNoise* dsp;
    dsp::UI ui;
    Vec vec;
};

struct OscSawtoothWord : public NativeWord {
    OscSawtoothWord() { dsp = nullptr; }
    virtual ~OscSawtoothWord();
    virtual void run(Stack& stack);

    NWORD_CREATOR_DEFINE_LR(OscSawtoothWord)
private:
    dsp::OscSawtooth* dsp;
    dsp::UI ui;
    Vec vec;
};

}}

#endif
