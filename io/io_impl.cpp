#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <sndfile.h>

#include "io/io_impl.hpp"
#include "io/RtMidi.h"

namespace lr { namespace io {

struct MatReader : public NativeWord {
    MatReader() {
        in_sf = nullptr;
    }

    virtual ~MatReader() {
        if ( in_sf != nullptr ) {
            sf_close(in_sf);
        }
    }

    virtual void run(Stack& stack) {
        const char* file_name = stack.pop_string();
        const int dim = stack.pop_number();

        if ( in_sf == nullptr) {
            const int sr = 16000;
            SF_INFO in_info = { sr, sr, dim, SF_FORMAT_MAT5 | SF_FORMAT_FLOAT | SF_ENDIAN_LITTLE, 0, 0};
            in_sf = sf_open(file_name, SFM_READ, &in_info);

            lr_assert(in_sf != nullptr , "Can't open mat file");
        }

        float buf[dim];
        int count = sf_read_float(in_sf, buf, dim);
        if ( count != dim) {
            sf_seek(in_sf, 0, SF_SEEK_SET);
            count = sf_read_float(in_sf, buf, dim);
            lr_assert( count == dim, "Can't read data from mat");
        }

        for ( int i = 0; i < dim; i++) {
            stack.push_number( buf[i] );
        }
    }

    NWORD_CREATOR_DEFINE_LR(MatReader)
private:
    SNDFILE* in_sf;
};

struct WavReader : public NativeWord {
    WavReader() {
        in_sf = nullptr;
    }

    virtual ~WavReader() {
        if ( in_sf != nullptr ) {
            sf_close(in_sf);
        }
    }

    virtual void run(Stack& stack) {
        const char* file_name = stack.pop_string();
        const size_t bs = stack.pop_number();

        if ( in_sf == nullptr) {
            SF_INFO in_info;
            memset (&in_info, 0, sizeof (in_info)) ;
            in_sf = sf_open(file_name, SFM_READ, &in_info);

            lr_assert(in_sf != nullptr, "Can't open wav file");
            lr_assert(in_info.channels == 1, "WavReader only support mono");

            vec = Vec::Zero(bs, 1);
        }

        lr_assert( vec.size() == (int)bs , "block size should be fixed");

        float buf[bs];
        size_t count = sf_read_float(in_sf, buf, bs);
        if ( count != bs) {
            sf_seek(in_sf, 0, SF_SEEK_SET);

            count = sf_read_float(in_sf, buf, bs);
            lr_assert( count == bs, "can't read target block size");
        }

        auto d = vec.data();
        for (size_t i = 0; i < bs; i++) {
            d[i] = buf[i];
        }

        stack.push_vector(&vec);
    }

    NWORD_CREATOR_DEFINE_LR(WavReader)
private:
    SNDFILE* in_sf;
    Vec vec;
};

struct WavWriter : public NativeWord {
    WavWriter() {
        out_sf = nullptr;
    }
    virtual ~WavWriter() {
        if ( out_sf != nullptr ) {
            sf_close(out_sf);
        }
    }

    virtual void run(Stack& stack) {
        const char* file_name = stack.pop_string();
        int sr = stack.pop_number();
        int ch = stack.pop_number();

        lr_assert(ch == 1, "Current only support mono mode!");

        if ( out_sf == nullptr) {
            SF_INFO out_info = { sr, sr, ch, SF_FORMAT_WAV | SF_FORMAT_FLOAT | SF_ENDIAN_LITTLE, 0, 0};
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

    NWORD_CREATOR_DEFINE_LR(WavWriter)
private:
    SNDFILE* out_sf;
};

struct MidiInWord : public NativeWord {
    MidiInWord() {
        midi_ = nullptr;
    }
    virtual ~MidiInWord() {
        if ( midi_ != nullptr) {
            delete midi_;
        }
    }

    virtual void run(Stack& stack) {
        int port = stack.pop_number();

        if ( midi_ == nullptr) {
            try {
                midi_ = new RtMidiIn();
                midi_->setCallback(&MidiInWord::midiHandler, (void*)this);
                midi_->openPort(port);
            }
            catch (RtMidiError& error) {
                error.printMessage();
                lr_panic("Open MIDI input error!");
            }
        }

        MidiMessage msg = MidiMessage::null();
        pop_message(msg);
        stack.push_number(msg.value_);
    }

    static void midiHandler(double timeStamp, std::vector<unsigned char>* bytes, void* ptr);

    NWORD_CREATOR_DEFINE_LR(MidiInWord)

private:
     // message handler
    void push_message(MidiMessage& message) {
        msg_mutex_.lock();
        msg_queue_.push(std::move(message));
        msg_mutex_.unlock();
    }

    int pop_message(MidiMessage& message) {
        if (msg_queue_.size() == 0) {
            return 0;
        }
        msg_mutex_.lock();
        message = msg_queue_.front();
        msg_queue_.pop();
        msg_mutex_.unlock();
        return 1;
    }

private:
    RtMidiIn* midi_;
    Vec out_;

    // MIDI control message
    std::mutex msg_mutex_;
    std::queue<io::MidiMessage> msg_queue_;
};

void MidiInWord::midiHandler(double timeStamp, std::vector<unsigned char>* bytes, void* ptr) {
    /*
    MidiInWord* wd = (MidiInWord *)ptr;
    if ( wd->out_.size() != bytes->size() ) {
        wd->out_ = Vec::Zero( bytes->size() , 1);
    }
    size_t sz = bytes->size();
    TNT* d = wd->out_.data();
    TNT* bd = bytes->data();
    for (size_t i = 0; i < sz; i++) {
        d[i] = bd[i];
    }
    */

    MidiInWord* wd = (MidiInWord *)ptr;
    lr::io::MidiMessage msg = lr::io::MidiMessage::parse(bytes->data(), bytes->size());
    wd->push_message(msg);
}

void init_words(Enviroment& env) {
    env.insert_native_word("io.write_wav", WavWriter::creator);
    env.insert_native_word("io.read_mat", MatReader::creator);
    env.insert_native_word("io.read_wav", WavReader::creator);

    env.insert_native_word("io.midi_in", MidiInWord::creator);
}

}}
