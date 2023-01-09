#ifndef _FAUST_REVERB_HPP_
#define _FAUST_REVERB_HPP_

#include "lr.hpp"
#include "faust/dsp.hpp"

namespace dsp {
    class ReFreeverb;
}

namespace lr { namespace faust {

struct ReFreeverbWord : public NativeWord {
    ReFreeverbWord() { dsp = nullptr; }
    virtual ~ReFreeverbWord();
    virtual void run(Stack& stack);

    NWORD_CREATOR_DEFINE_LR(ReFreeverbWord)

private:
    dsp::ReFreeverb* dsp;
    Vec out;
};

}}

#endif
