#ifndef _FAUST_IMPL_HPP_
#define _FAUST_IMPL_HPP_

#include "lr.hpp"
#include "faust/osc.hpp"
#include "faust/reverb.hpp"

namespace lr { namespace faust {

void init_words(Enviroment& env) {
    env.insert_native_word("faust.osc.sine", OscSineWord::creator);
    env.insert_native_word("faust.osc.sawtooth", OscSawtoothWord::creator);
    env.insert_native_word("faust.osc.square", OscSquareWord::creator);
    env.insert_native_word("faust.osc.triangle", OscTriangleWord::creator);

    env.insert_native_word("faust.no.white", NoiseWhiteWord::creator);

    env.insert_native_word("faust.re.freeverb", ReFreeverbWord::creator);
}

}}


#endif
