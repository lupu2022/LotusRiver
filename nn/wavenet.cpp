#include <Eigen/Core>
#include <Eigen/StdVector>

#include "lr.hpp"
#include "nn/wavenet.hpp"

namespace lr { namespace wavenet {

typedef TNT (* activationFunction)(TNT x);
void applyActivation(TNT *data, size_t rows, size_t cols, activationFunction activation) {
    for (size_t i = 0; i < rows*cols; ++i) {
            data[i] = activation(data[i]);
    }
}

typedef TNT (* gatedActivationFunction)(TNT x1, TNT x2);
void applyGatedActivation(TNT *data, size_t rows, size_t cols, gatedActivationFunction activation) {

    size_t rowsHalf = rows / 2;
    for (size_t row = 0; row < rowsHalf; ++row) {
        size_t startIdx1 = row * cols + 0;
        size_t startIdx2 = (row+rowsHalf) * cols + 0;
        for (size_t col = 0; col < cols; ++col)
            data[startIdx1+col] = activation(data[startIdx1+col], data[startIdx2+col]);
    }
}

TNT tanh(TNT x) {
    return tanhf(x);
}

TNT sigmoid(TNT x) {
    return 1.0f / (1.0f + expf(-x));
}

TNT relu(TNT x) {
    if (x < 0.0f)
        return 0.0f;
    else
        return x;
}

TNT softsign(TNT x) {
    return x / (1.0f + fabsf(x));
}

TNT linear(TNT x) {
    return x;
}

TNT gated(TNT x1, TNT x2) {
    return tanh(x1)*sigmoid(x2);
}

TNT softgated(TNT x1, TNT x2) {
    return softsign(x1) * softsign(x2);
}

void tanh(TNT* data, size_t rows, size_t cols) {
    applyActivation(data, rows, cols, (activationFunction)tanh);
}
void sigmoid(TNT* data, size_t rows, size_t cols) {
    applyActivation(data, rows, cols, (activationFunction)sigmoid);
}
void relu(TNT* data, size_t rows, size_t cols) {
    applyActivation(data, rows, cols, (activationFunction)relu);
}
void softsign(TNT* data, size_t rows, size_t cols) {
    applyActivation(data, rows, cols, (activationFunction)softsign);
}
void linear(TNT* data, size_t rows, size_t cols) {
    return;
}
void gated(TNT* data, size_t rows, size_t cols) {
    assert(rows % 2 == 0);
    applyGatedActivation(data, rows, cols, (gatedActivationFunction)gated);
}
void softgated(TNT* data, size_t rows, size_t cols) {
    assert(rows % 2 == 0);
    applyGatedActivation(data, rows, cols, (gatedActivationFunction)softgated);
}

bool isGated(std::string name) {
    if ((name == "gated") || (name == "softgated"))
        return true;
    return false;
}

activationFuncArray getActivationFuncArray(std::string name) {
    if (name == "tanh")
        return tanh;
    else if (name == "sigmoid")
        return sigmoid;
    else if (name == "relu")
        return relu;
    else if (name == "softsign")
        return softsign;
    else if (name == "linear")
        return linear;
    else if (name == "gated")
        return gated;
    else if (name == "softgated")
        return softgated;
    else
        throw std::invalid_argument("Received unkown activation name.");
}

Convolution::Convolution(size_t inputChannels,
                         size_t outputChannels,
                         int filterWidth,
                         int dilation) :
                         bias(outputChannels),
                         outVec(outputChannels),
                         pos(0),
                         dilation(dilation),
                         inputChannels(inputChannels),
                         outputChannels(outputChannels),
                         filterWidth(filterWidth) {
    resetFifo();
    resetKernel();
}

void Convolution::resetKernel() {
    kernel.clear();
    kernel.reserve(filterWidth);
    for (int i = 0; i < filterWidth; ++i) {
        Eigen::MatrixXf x(inputChannels, outputChannels);
        x.setZero();
        kernel.push_back(x);
    }
    bias = Eigen::RowVectorXf (outputChannels);
    bias.setZero();
}

void Convolution::resetFifo() {
    memory.clear();
    memory.reserve(getFilterOrder());
    for (int i = 0; i < getFilterOrder(); ++i) {
        Eigen::RowVectorXf x(inputChannels);
        x.setZero();
        memory.push_back(x);
    }
    pos = 0;
}

void Convolution::setParams(size_t newInputChannels, size_t newOutputChannels,
                            int newFilterWidth, int newDilation) {
    inputChannels = newInputChannels;
    outputChannels = newOutputChannels;
    filterWidth = newFilterWidth;
    dilation = newDilation;
    outVec = Eigen::RowVectorXf (outputChannels);
    resetFifo();
    resetKernel();
}

int Convolution::getFilterOrder() const {
    return (filterWidth-1)*dilation + 1;
}

void Convolution::process(TNT* data, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        processSingleSample(data, i , numSamples);
    }
}

void Convolution::processSingleSample(TNT* data, int i, int numSamples) {
    if ( (int)memory.size() != getFilterOrder())
        resetFifo();
    auto fifo = memory.begin();
    for (size_t ch = 0; ch < inputChannels; ++ch)
        (*(fifo+pos))[ch] = data[idx(ch, i, numSamples)];
    outVec.setZero();
    std::vector<Eigen::MatrixXf>::iterator it;
    int j = 0;
    for (auto it = kernel.begin(); it != kernel.end(); it++)
    {
        int readPos = mod((pos - j * dilation), getFilterOrder());
        outVec = outVec + *(fifo+readPos) * (*it);
        j += 1;
    }
    outVec = outVec + bias;
    for (size_t ch = 0; ch < outputChannels; ++ch)
        data[idx(ch, i, numSamples)] = outVec[ch];
    pos = mod(pos + 1, getFilterOrder());
}


int Convolution::mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

int Convolution::idx(int ch, int i, int numSamples) {
    return ch * numSamples + i;
}

void Convolution::setWeight(std::vector<TNT> W, std::string name) {
    if (name == "W")
        setKernel(W);
    else if (name == "b")
        setBias(W);
}

void Convolution::setKernel(std::vector<TNT> W) {
    assert(W.size() == inputChannels*outputChannels*filterWidth);
    size_t i = 0;
    for (int k = 0; k < filterWidth; ++k)
        for(size_t row = 0; row < inputChannels; ++row)
            for (size_t col = 0; col < outputChannels; ++col)
            {
                kernel[filterWidth-1-k](row,col) = W[i];
                i += 1;
            }
}

void Convolution::setBias(std::vector<TNT> W) {
    assert(W.size() == outputChannels);
    for (size_t i = 0; i < outputChannels; ++i)
        bias(i) = W[i];
}

ConvolutionLayer::ConvolutionLayer(size_t inputChannels,
                                   size_t outputChannels,
                                   int filterWidth,
                                   int dilation,
                                   bool residual,
                                   std::string activationName):
                                   conv(inputChannels,
                                        isGated(activationName) ? outputChannels * 2 : outputChannels,
                                        filterWidth,
                                        dilation),
                                   out1x1(outputChannels, outputChannels, 1, 1),
                                   residual(residual),
                                   usesGating(isGated(activationName)),
                                   activation(getActivationFuncArray(activationName)) {
}

void ConvolutionLayer::process(float* data, int numSamples) {
    conv.process(data, numSamples);
    activation(data, conv.getNumOutputChannels(), numSamples);
    if (residual) {
        out1x1.process(data, numSamples);
    }
}

void ConvolutionLayer::process(float* data, float* skipData, int numSamples) {
    conv.process(data, numSamples);
    activation(data, conv.getNumOutputChannels(), numSamples);
    copySkipData(data, skipData, numSamples);
    if (residual) {
        out1x1.process(data, numSamples);
    }
}

void ConvolutionLayer::copySkipData(float *data, float *skipData, int numSamples) {
    size_t skipChannels = usesGating ? conv.getNumOutputChannels()/2 : conv.getNumOutputChannels();
    for (size_t i = 0; i < (size_t)numSamples*skipChannels; ++i)
        skipData[i] = data[i];
}

void ConvolutionLayer::setParams(size_t newInputChannels, size_t newOutputChannels,
                                 int newFilterWidth, int newDilation, bool newResidual,
                                 std::string newActivationName) {
    activation = getActivationFuncArray(newActivationName);
    usesGating = isGated(newActivationName);
    size_t internalChannels = usesGating ? newOutputChannels * 2 : newOutputChannels;
    conv.setParams(newInputChannels, internalChannels, newFilterWidth, newDilation);
    out1x1.setParams(newOutputChannels, newOutputChannels, 1, 1);
    residual = newResidual;
}

void ConvolutionLayer::setWeight(std::vector<float> W, std::string name) {
    if ((name == "W_conv") || (name == "W"))
        conv.setWeight(W, "W");
    else if ((name == "b_conv") || (name == "b"))
        conv.setWeight(W, "b");
    else if (name == "W_out")
        out1x1.setWeight(W, "W");
    else if (name == "b_out")
        out1x1.setWeight(W, "b");
}

}}
