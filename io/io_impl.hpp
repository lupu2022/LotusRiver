#ifndef _IO_IMPL_HPP_
#define _IO_IMPL_HPP_

#include <sndfile.h>
#include "lr.hpp"

namespace lr { namespace io {

struct MonoWavOut : public NativeWord {
    MonoWavOut() {
        out_sf = nullptr;
    }
    ~MonoWavOut() {
        sf_close(out_sf);
    }


    virtual void run(Stack& stack) {
        const char* file_name = stack.pop_string();
        int sr = stack.pop_number();

        if ( out_sf == nullptr) {
            SF_INFO out_info = { sr, sr, 1, SF_FORMAT_WAV | SF_FORMAT_FLOAT | SF_ENDIAN_LITTLE, 0, 0};
            out_sf = sf_open(file_name, SFM_WRITE, &out_info);
        }

        if ( stack.top().is_number() ) {
            float v = stack.pop_number();
            sf_write_float(out_sf, &v, 1);
            return;
        }

        auto v = stack.pop_vector();
        auto s = v->size();
        sf_write_float(out_sf, v->data(), s);
    }
    NWORD_CREATOR_DEFINE_LR(MonoWavOut)
private:
    SNDFILE* out_sf;
};

void init_words(Enviroment& env) {
    env.insert_native_word("io.mono_wav", MonoWavOut::creator);
}

}}
#endif
