#ifndef _IO_IMPL_HPP_
#define _IO_IMPL_HPP_

#include <sndfile.h>
#include "lr.hpp"

namespace lr { namespace io {

struct PerfReader : public NativeWord {
    PerfReader() {
        in_sf = nullptr;
    }

    virtual ~PerfReader() {
        if ( in_sf != nullptr ) {
            sf_close(in_sf);
        }
    }

    virtual void run(Stack& stack) {
        const char* file_name = stack.pop_string();

        if ( in_sf == nullptr) {
            const int sr = 16000;
            SF_INFO in_info = { sr, sr, 3, SF_FORMAT_MAT5 | SF_FORMAT_FLOAT | SF_ENDIAN_LITTLE, 0, 0};
            in_sf = sf_open(file_name, SFM_READ, &in_info);
        }

        float buf[3];
        int count = sf_read_float(in_sf, buf, 3);
        if ( count != 3) {
            sf_seek(in_sf, 0, SF_SEEK_SET);
            stack.push_number(0);
            stack.push_number(0);
            return;
        }

        float p = buf[0];
        //float c = buf[1];
        float l = buf[2];

        stack.push_number(p);
        stack.push_number(l);
    }

    NWORD_CREATOR_DEFINE_LR(PerfReader)
private:
    SNDFILE* in_sf;
};


struct MonoWavOut : public NativeWord {
    MonoWavOut() {
        out_sf = nullptr;
    }
    virtual ~MonoWavOut() {
        if ( out_sf != nullptr ) {
            sf_close(out_sf);
        }
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
    env.insert_native_word("io.read_perf", PerfReader::creator);
}

}}
#endif
