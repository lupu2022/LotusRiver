#include "faust/dsp.hpp"
#include "faust/auto/osc~.hpp"
#include "faust/auto/noise~.hpp"
#include "faust/auto/sawtooth~.hpp"

#include "lr.hpp"
#include "faust/osc.hpp"

namespace lr { namespace faust {

OscSineWord::~OscSineWord() {
    if ( dsp != nullptr ) {
        delete dsp;
    }
}
void OscSineWord::run(Stack& stack) {
    int sr = stack.pop_number();
    int bs = stack.pop_number();
    TNT freq = stack.pop_number();
    if ( dsp == nullptr) {
        dsp = new dsp::OscSine();
        dsp->init(sr);
        dsp->buildUserInterface(&ui);

        vec = Vec::Zero(bs, 1);
    }

    *ui.freq = freq;
    TNT* d = vec.data();
    dsp->compute(bs, nullptr, &d);

    stack.push_vector(&vec);
}


OscNoiseWord::~OscNoiseWord() {
    if ( dsp != nullptr ) {
        delete dsp;
    }
}

void OscNoiseWord::run(Stack& stack) {
    int bs = stack.pop_number();
    if ( dsp == nullptr) {
        dsp = new dsp::OscNoise();
        dsp->init(44100);
        dsp->buildUserInterface(&ui);
        vec = Vec::Zero(bs, 1);
    }

    TNT* d = vec.data();
    dsp->compute(bs, nullptr, &d);

    stack.push_vector(&vec);
}

OscSawtoothWord::~OscSawtoothWord() {
    if ( dsp != nullptr ) {
        delete dsp;
    }
}

void OscSawtoothWord::run(Stack& stack) {
    int sr = stack.pop_number();
    int bs = stack.pop_number();
    TNT freq = stack.pop_number();
    if ( dsp == nullptr) {
        dsp = new dsp::OscSawtooth();
        dsp->init(sr);
        dsp->buildUserInterface(&ui);

        vec = Vec::Zero(bs, 1);
    }

    *ui.freq = freq;
    TNT* d = vec.data();
    dsp->compute(bs, nullptr, &d);

    stack.push_vector(&vec);
}

}}
