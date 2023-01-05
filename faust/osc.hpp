#ifndef _FAUST_OSC_HPP_
#define _FAUST_OSC_HPP_

#include "lr.hpp"

namespace dsp {
    class OscSine;
    class OscNoise;
    class OscSawtooth;
}

namespace lr { namespace faust {

struct OscSineWord : public NativeWord {
    OscSineWord() { }
    virtual ~OscSineWord() { }

    virtual void run(Stack& stack);

    NWORD_CREATOR_DEFINE_LR(OscSineWord)
private:
    dsp::OscSine* dsp;
};

struct OscNoiseWord : public NativeWord {
    OscNoiseWord() { }
    virtual ~OscNoiseWord() { }

    virtual void run(Stack& stack);

    NWORD_CREATOR_DEFINE_LR(OscNoiseWord)
private:
    dsp::OscNoise* dsp;
};

struct OscSawtoothWord : public NativeWord {
    OscSawtoothWord() { }
    virtual ~OscSawtoothWord() { }

    virtual void run(Stack& stack);

    NWORD_CREATOR_DEFINE_LR(OscSawtoothWord)
private:
    dsp::OscSawtooth* dsp;
};

}}

#endif
