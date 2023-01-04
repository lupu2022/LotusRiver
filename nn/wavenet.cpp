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


// help functions
float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

void InputLayer::process(const TNT* data, size_t number) {
    if ( out_.size() < number *  kernel_.size() ) {
        out_.resize(number * kernel_.size(), 0.0 );
    }

    // change input from 1 channel to multipule channels
    for (size_t i = 0; i < number; i++) {
        TNT* target = out_.data() + i * kernel_.size();

        for (size_t j = 0; j < kernel_.size(); j++) {
            target[j] = kernel_[j] * data[i] + bias_[j];
        }
    }
}

void HiddenLayer::process(const std::vector<TNT>& data, size_t number) {
    lr_assert(data.size() == number * channels_, "Input must has same channels with define");

    // preparing memory for skip and next out
    if ( out_.size() < number * channels_ ) {
        out_.resize(number * channels_, 0.0);
    }

    const TNT* sample = data.data();
    for ( size_t i = 0; i < number; i++) {
        processOneSample(sample, i);
        sample = sample + channels_;
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
                out = out + x[k] * w[j + k * kernel_size_];
            }
        }
        out = out + *b;
        gate_out_[i] = out;

    }

    // 2. gated activation
    for (size_t i = 0; i < channels_; i++) {
        TNT o1 = tanh( gate_out_[i]);
        TNT o2 = sigmoid( gate_out_[i + channels_]);

        out_[t * channels_ + i ] = o1 * o2;
    }
}

const TNT* HiddenLayer::fifo_get(size_t kernel) {
    int pos = (int)fifo_cursor_ + kernel * dialation_;
    pos = pos % (int)fifo_order_;

    return fifo_.data() + pos * channels_;
}


void ResLayer::process(const std::vector<TNT>& data, const std::vector<TNT>& gateOut, size_t number) {
    lr_assert(data.size() == number * channels_ , "input size is wrong");
    lr_assert(gateOut.size() == number * channels_, "input size is wrong");

    if ( out_.size() < number *  channels_ ) {
        out_.resize(number * channels_, 0.0);
    }

    for ( size_t t = 0; t < number; t++) {
        for (size_t i = 0; i < channels_; i++) {
            TNT out = 0.0;
            for (size_t j = 0; j < channels_; j++) {
                out = out + gateOut[t * channels_ + j] * kernel_[i * channels_ + j];
            }
            out = out + bias_[i];
            out_[t * channels_ + i] = out + data[t * channels_ + i];
        }
    }
}


void MixerLayer::process(const std::vector<TNT>& gateOut, const size_t layer, const size_t length) {
    if ( layer == 0 ) {
        out_.resize( length, bias_[0] );
    }

    lr_assert( length == out_.size(), "output must has same size");
    for(size_t t = 0; t < length; t++) {
        TNT out = 0.0;
        for (size_t i = 0; i < channels_; i++) {
            out = out + gateOut[t * channels_ + i] * kernel_[layer * channels_ + i];
        }
        out_[t] = out_[t] + out;
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

    current_weight_ = "filter.linear_mix.";
    mixer_ = new MixerLayer(channels_, dialations_.size(), this);

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
    while (getline( wfile, line)) {
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

    if ( vec.size() > 0 ) {
        weights_[name] = vec;
    }
}

void WaveNet::process(const TNT* data, size_t length) {
    input_->process(data, length);

    auto out = input_->output();
    for (size_t i = 0; i < dialations_.size(); i++) {
        hiddens_[i]->process( out, length);

        mixer_->process( hiddens_[i]->output(), i, length);

        residuals_[i]->process( out, hiddens_[i]->output(), length);
        out = residuals_[i]->output();
    }

    out = mixer_->output();

    for (size_t t = 0; t < length; t++) {
        std::cout << out[t] << std::endl;
    }

    exit(0);
}

}}}
