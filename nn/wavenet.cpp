#include <fstream>
#include <iostream>
#include <sstream>
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

namespace lr { namespace nn { namespace wavenet {

void InputLayer::process(const TNT* data, size_t number) {
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

void HiddenLayer::process(const std::vector<TNT>& data, size_t number, size_t channels) {
    lr_assert(channels == channels_, "Input must has same channels with define");
    lr_assert(data.size() == number * channels, "Input must has same channels with define");

    // preparing memory for skip and next out
    if ( skip_out_.size() < number * channels_ ) {
        skip_out_.resize(0.0, number * channels_);
    }

    for ( size_t i = 0; i < number; i++) {
        const TNT* sample = data.data() + i * channels;
        processOneSample(sample, i);
    }
}

void HiddenLayer::processOneSample(const TNT* sample, size_t t) {
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

    // 2. gated activation
    for (size_t i = 0; i < channels_; i++) {
        TNT o1 = tanh( gate_out_[i]);
        TNT o2 = sigmoid( gate_out_[i + channels_]);

        gate_out_[i] = o1 * o2;
        skip_out_[t * channels_ + i ] = gate_out_[i];
    }

}

void ResLayer::process(const TNT* data, const TNT* gateOut, size_t number) {
    if ( out_.size() < number *  channels_ ) {
        out_.resize(0.0, number * channels_ );
    }

    for ( size_t t = 0; t < number; t++) {
        for (size_t i = 0; i < channels_; i++) {
            TNT out = 0.0;
            for (size_t j = 0; j < channels_; j++) {
                out = out + gateOut[t * channels_ + j] * kernel_[i * channels_ + j];
            }
            out = out + bias_[i] + data[t * channels_ + i];
            out_[t * channels_ + i] = out;
        }
    }
}


WaveNet::~WaveNet() {

}

void WaveNet::init() {
    current_weight_ = "filter.input_layer.";
    input_ = new InputLayer(channels_, this);

    for (size_t i = 0; i < dialations_.size(); i++) {
        std::stringstream ss;
        ss << "filter.hidden." << i << ".";
        current_weight_ = ss.str();

        auto hidden = new  HiddenLayer(channels_, dialations_[i], kernel_size_, this);
        hiddens_.push_back( hidden );
    }

    for (size_t i = 0; i < dialations_.size(); i++) {
        std::stringstream ss;
        ss << "filter.residuals." << i << ".";
        current_weight_ = ss.str();

        auto hidden = new  ResLayer(channels_, this);
        residuals_.push_back( hidden );
    }

    current_weight_ = "";
}

void WaveNet::new_weight(std::vector<TNT>& w, std::vector<TNT>& b) {
    lr_assert(current_weight_ != "", "target vector name error");

    std::string wname = current_weight_ + "weight";
    std::vector<TNT>& w_ = weights_[ wname ];
    lr_assert(w_.size() == w.size(), " weight vector must has same size");
    w.assign(w_.begin(), w_.end());

    std::string bname = current_weight_ + "bias";
    std::vector<TNT>& b_ = weights_[ bname ];
    lr_assert(b_.size() == b.size(), " weight vector must has same size");
    b.assign(b_.begin(), b_.end());
}

void WaveNet::load_weight(const char* file_name) {
    std::ifstream wfile(file_name);
    lr_assert(wfile.is_open(), "Can't open weight file");

    std::string line;
    std::string name;
    std::vector<TNT> vec;
    while (getline( wfile, line)){
        if ( line.find("- ") == 0) {
            if ( vec.size() > 0 ) {
                weights_[name] = vec;
                vec.clear();
            }
            name = line.substr(2, line.size() - 3);
        } else if ( line.find("  - ") == 0 ) {
            std::stringstream ss(line.substr(4));
            TNT v;
            ss >> v;
            vec.push_back(v);
        }
    }
}

}}}
