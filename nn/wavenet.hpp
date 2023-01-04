#ifndef _NN_CONV_HPP_
#define _NN_CONV_HPP_

#include <map>
#include <vector>
#include "lr.hpp"

namespace lr { namespace nn { namespace wavenet {

struct ParameterRegister {
    virtual void new_weight(std::vector<TNT>& w, std::vector<TNT>& b) = 0;
};

struct InputLayer {
    InputLayer(const size_t out_channels, ParameterRegister* reg) {
        kernel_.resize(out_channels);
        bias_.resize(out_channels);

        reg->new_weight(kernel_, bias_);
    }

    void process(const TNT* data, size_t number);

    const std::vector<TNT>& output() {
        return out_;
    }

private:
    std::vector<TNT> kernel_;
    std::vector<TNT> bias_;
    std::vector<TNT> out_;
};

struct HiddenLayer {
    HiddenLayer(const size_t channels,
                const size_t dialation,
                const size_t kernel_size,
                ParameterRegister* reg) :
        channels_(channels), dialation_(dialation), kernel_size_(kernel_size) ,
        fifo_order_( (kernel_size - 1) * dialation + 1) {

        gate_kernel_.resize(2 * channels * channels * kernel_size);
        gate_bias_.resize(2 * channels);
        gate_out_.resize(2 * channels);

        fifo_.resize(fifo_order_ * channels, 0.0);
        fifo_cursor_ = 0;

        reg->new_weight(gate_kernel_, gate_bias_);
    }

    void process(const std::vector<TNT>& data, size_t number);
    const std::vector<TNT>& output() {
        return out_;
    }

private:
    void processOneSample(const TNT* sample, size_t t);
    const TNT* fifo_get(size_t kernel);

private:
    const size_t channels_;
    const size_t dialation_;
    const size_t kernel_size_;
    const size_t fifo_order_;           // (kernel_size - 1) * dialation + 1

    std::vector<TNT> gate_kernel_;      // output channel * input channel * kernel size
    std::vector<TNT> gate_bias_;        // output channel
    std::vector<TNT> gate_out_;         // output channel

    std::vector<TNT> res_kernel_;       // channel * channel * 1
    std::vector<TNT> res_bias_;         // channel

    // input
    std::vector<TNT> fifo_;             // fifo_order * channel
    size_t fifo_cursor_;

    // output
    std::vector<TNT> out_;
};

struct ResLayer {
    ResLayer(const size_t channels, ParameterRegister* reg) : channels_(channels) {
        kernel_.resize(channels * channels);
        bias_.resize(channels);

        reg->new_weight(kernel_, bias_);
    }

    void process(const TNT* data, const TNT* gateOut, size_t number);
    const std::vector<TNT>& output() {
        return out_;
    }

private:
    const size_t channels_;
    std::vector<TNT> kernel_;
    std::vector<TNT> bias_;
    std::vector<TNT> out_;
};

struct WaveNet : public ParameterRegister {
    WaveNet (size_t channels, size_t kernel_size, const std::vector<size_t>& dialations, const char* weight_file):
        channels_(channels), kernel_size_(kernel_size), dialations_(dialations) {

        load_weight(weight_file);
        init();
    }
    virtual ~WaveNet();
    virtual void new_weight(std::vector<TNT>& w, std::vector<TNT>& b);

    void process(const TNT* data, size_t length);

private:
    void load_weight(const char* file_name);
    void init();

private:
    size_t  channels_;
    size_t  kernel_size_;
    std::vector<size_t> dialations_;

    std::string current_weight_;
    std::map<const std::string, std::vector<TNT>> weights_;

    InputLayer* input_;
    std::vector<HiddenLayer*> hiddens_;
    std::vector<ResLayer*> residuals_;
};

struct WaveNetWord : public lr::NativeWord {
    WaveNetWord() {
        net_ = nullptr;
    }
    virtual ~WaveNetWord() {
        if ( net_ != nullptr ) {
            delete net_;
        }
    }

    virtual void run(Stack& stack) {
        const char* file_name = stack.pop_string();
        size_t repeat = stack.pop_number();
        size_t dialation = stack.pop_number();
        size_t kernel_size = stack.pop_number();
        size_t channels = stack.pop_number();

        if ( net_ == nullptr) {
            std::vector<size_t> ds;
            for(size_t r = 0; r < repeat; r++) {
                for(size_t i = 0; i < dialation; i++) {
                    ds.push_back( 1 << i );
                }
            }
            net_ = new WaveNet(channels, kernel_size, ds, file_name);
        }

        auto v = stack.pop_vector();
        net_->process(v->data(), v->size());
    }

    NWORD_CREATOR_DEFINE_LR(WaveNetWord)
private:
    WaveNet* net_;
};


}}}


#endif
