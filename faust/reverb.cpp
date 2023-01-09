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
    auto vin = stack.pop_vector();

    if ( dsp == nullptr) {
        dsp = new dsp::ReFreeverb();
        dsp->init(sr);
    }

    if ( out.size() < vin->size() ) {
        out = Vec::Zero( vin->size(), 1);
    }

    TNT* din = const_cast<TNT *>(vin->data());
    TNT* dout = out.data();
    dsp->compute(vin->size(), &din, &dout);

    stack.push_vector(&out);
}

}}
