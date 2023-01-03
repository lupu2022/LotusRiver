#ifndef _NN_IMPL_HPP_
#define _NN_IMPL_HPP_

#include "lr.hpp"
#include "nn/wavenet.hpp"

namespace lr { namespace nn {

void init_words(Enviroment& env) {
    env.insert_native_word("nn.wavenet", wavenet::WaveNetWord::creator);
}

}}

#endif
