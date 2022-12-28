#include <Eigen/Core>
#include <Eigen/StdVector>

#include "lr.hpp"
#include "nn/wavenet.hpp"

/*
def forward(self, x):
    out = x
    skips = []
    out = self.input_layer(out)

    for hidden, residual in zip(self.hidden, self.residuals):
        x = out
        out_hidden = hidden(x)

        # gated activation
        #   split (32,16,3) into two (16,16,3) for tanh and sigm calculations
        out_hidden_split = torch.split(out_hidden, self.num_channels, dim=1)
        out = torch.tanh(out_hidden_split[0]) * torch.sigmoid(out_hidden_split[1])

        skips.append(out)

        out = residual(out)
        out = out + x[:, :, -out.size(2) :]

    # modified "postprocess" step:
    out = torch.cat([s[:, :, -out.size(2) :] for s in skips], dim=1)
    out = self.linear_mix(out)
    return out
*/

namespace lr { namespace wavenet {

struct InputLayer {
    InputLayer(const size_t out_channels) {
        kernel_.resize(0.0, out_channels);
        bias_.resize(0.0, out_channels);
    }

    void process(const TNT* data, size_t number) {
        if ( out_.size() < number *  kernel_.size() ) {
            out_.resize(0.0, number * kernel_.size() );
        }

        // change input from 1 channel to multipule channels
        #pragma omp parallel
        for (size_t i = 0; i < number; i++) {
            TNT* target = out_.data() + i * kernel_.size();

            for (size_t j = 0; j < kernel_.size(); j++) {
                target[j] = kernel_[j] * data[i] + bias_[j];
            }
        }
    }

    TNT* output() {
        return out_.data();
    }
private:
    std::vector<TNT> kernel_;
    std::vector<TNT> bias_;
    std::vector<TNT> out_;
};

// main computing stuff
struct HiddenLayer {
    HiddenLayer(const size_t channels, const size_t dialation, const size_t kernel_size)
        : channels_(channels), dialation_(dialation), kernel_size_(kernel_size) ,
          fifo_order_( (kernel_size - 1) * dialation + 1) {

        gate_kernel_.resize(0.0, 2 * channels * channels * kernel_size);
        gate_bias_.resize(0.0, 2 * channels);
        gate_out_.resize(0.0, 2 * channels);

        res_kernel_.resize(0.0,  channels * channels );
        res_bias_.resize(0.0, channels);


        fifo_.resize(0.0, fifo_order_ * channels );
        fifo_cursor_ = 0;
    }

    void process(const TNT* data, size_t number, size_t channels) {
        lr_assert(channels == channels_, "Input must has same channels with define");

        // preparing memory for skipout and out


        for ( size_t i = 0; i < number; i++) {
            const TNT* sample = data + i * channels;
            processOneSample(sample, i);
        }
    }

private:
    void processOneSample(const TNT* sample, size_t t) {
        // 0. update fifo
        for (size_t i = 0; i < channels_; i++) {
            fifo_[fifo_cursor_ * channels_ + i] = sample[i];
        }
        fifo_cursor_ ++;
        if ( fifo_cursor_ >= fifo_order_ ) {
            fifo_cursor_ = 0;
        }

        // 1. update gate conv ouput
        for (size_t i = 0; i < channels_ * 2; i++) {
            const TNT* w = gate_kernel_.data() + i * channels_ * kernel_size_;
            const TNT* b = gate_bias_.data() + i;

            TNT out = 0.0;
            for (size_t j = 0; j < kernel_size_; j++) {
                const TNT* x = fifo_get(j);
                for (size_t k = 0; k < channels_; k++) {
                    out = out + x[k] * w[j * channels_ + k];
                }
            }
            out = out + *b;

            gate_out_[i] = out;
        }

        // gated activation
        for (size_t i = 0; i < channels_; i++) {
            TNT o1 = tanh( gate_out_[i]);
            TNT o2 = sigmoid( gate_out_[i + channels_]);

            TNT out = o1 * o2;

        }

    }

    // help functions
    float sigmoid(float x) {
        return 1.0f / (1.0f + expf(-x));
    }

    const TNT* fifo_get(size_t kernel) {
        int pos = (int)fifo_cursor_ - 1 - kernel;
        pos = pos % (int)fifo_order_;
        if ( pos < 0 ) {
            pos = pos + (int)fifo_order_;
        }
        return fifo_.data() + pos * channels_;
    }

private:
    const size_t channels_;
    const size_t dialation_;
    const size_t kernel_size_;
    const size_t fifo_order_;           // (kernel_size - 1) * dialation + 1

    std::vector<TNT> gate_kernel_;      // 2 * channels * channels * kernel_size
    std::vector<TNT> gate_bias_;        // 2 * channels
    std::vector<TNT> gate_out_;

    std::vector<TNT> res_kernel_;       // channel * channel * 1
    std::vector<TNT> res_bias_;         // channel

    // input
    std::vector<TNT> fifo_;             // fifo_order * channel
    size_t fifo_cursor_;

    // output

};


}}
