#ifndef _FAUST_OSC_HPP_
#define _FAUST_OSC_HPP_

#include "lr.hpp"
#include "faust/dsp.hpp"

namespace dsp {
    class OscSine;
    class OscSawtooth;
    class OscSquare;

    class NoiseWhite;
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

struct OscSquareWord : public NativeWord {
    OscSquareWord() { dsp = nullptr; }
    virtual ~OscSquareWord();
    virtual void run(Stack& stack);

    NWORD_CREATOR_DEFINE_LR(OscSquareWord)
private:
    dsp::OscSquare* dsp;
    dsp::UI ui;
    Vec vec;
};


struct NoiseWhiteWord : public NativeWord {
    NoiseWhiteWord() { dsp = nullptr; }
    virtual ~NoiseWhiteWord();
    virtual void run(Stack& stack);

    NWORD_CREATOR_DEFINE_LR(NoiseWhiteWord)
private:
    dsp::NoiseWhite* dsp;
    dsp::UI ui;
    Vec vec;
};


}}

#endif
