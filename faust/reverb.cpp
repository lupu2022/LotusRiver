#include "faust/dsp.hpp"
#include "faust/auto/freeverb~.hpp"

#include "lr.hpp"
#include "faust/reverb.hpp"

namespace lr { namespace faust {

ReFreeverbWord::~ReFreeverbWord() {
    if ( dsp != nullptr ) {
        delete dsp;
    }
}
void ReFreeverbWord::run(Stack& stack) {
    int sr = stack.pop_number();

    if ( dsp == nullptr) {
        dsp = new dsp::ReFreeverb();
        dsp->init(sr);
    }

}

}}
