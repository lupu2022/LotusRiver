#ifndef _FAUST_IMPL_HPP_
#define _FAUST_IMPL_HPP_

#include "lr.hpp"
#include "faust/osc.hpp"

namespace lr { namespace faust {

void init_words(Enviroment& env) {
    env.insert_native_word("fasut.osc.sine", OscSineWord::creator);
    env.insert_native_word("fasut.osc.noise", OscNoiseWord::creator);
    env.insert_native_word("fasut.osc.sawtooth", OscSawtoothWord::creator);
}

}}


#endif
