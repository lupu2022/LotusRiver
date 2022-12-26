#ifndef _NN_CONV_HPP_
#define _NN_CONV_HPP_

#include <Eigen/Core>
#include <Eigen/StdVector>
#include "lr.hpp"

namespace lr { namespace wavenet {

typedef void (* activationFuncArray)(TNT *data, size_t rows, size_t cols);

struct Convolution {
public:
    Convolution(size_t inputChannels, size_t outputChannels, int filterWidth, int dilation = 1);
    int getFilterOrder() const;
    void process(TNT* data, int numSamples);
    void setParams(size_t inputChannels, size_t outputChannels, int filterWidth, int dilation);
    size_t getNumInputChannels() { return inputChannels; }
    size_t getNumOutputChannels() { return outputChannels; }
    void setWeight(std::vector<TNT> W, std::string name);

private:
    std::vector<Eigen::MatrixXf, Eigen::aligned_allocator<Eigen::MatrixXf >> kernel;
    Eigen::RowVectorXf bias;
    std::vector<Eigen::RowVectorXf, Eigen::aligned_allocator<Eigen::RowVectorXf> > memory;
    Eigen::RowVectorXf outVec;
    int pos;
    int dilation;
    size_t inputChannels;
    size_t outputChannels;
    int filterWidth;

    void resetFifo();
    void resetKernel();
    void processSingleSample(TNT* data, int i, int numSamples);

    int mod(int a, int b);
    int idx(int ch, int i, int numSamples);

    void setKernel(std::vector<TNT> W);
    void setBias(std::vector<TNT> W);
};

class ConvolutionLayer {
public:
    ConvolutionLayer(size_t inputChannels,
                     size_t outputChannels,
                     int filterWidth,
                     int dilation = 1,
                     bool residual = false,
                     std::string activationName = "linear");
    void process(float* data, int numSamples);
    void process(float* data, float* skipdata, int numSamples);
    void setParams(size_t newInputChannels, size_t newOutputChannels, int newFilterWidth,
                   int newDilation, bool newResidual, std::string newActivationName);
    void setWeight(std::vector<float> W, std::string name);

private:
    Convolution conv;
    Convolution out1x1;
    bool residual;
    bool usesGating;

    activationFuncArray activation;
    void copySkipData(float *data, float *skipData, int numSamples);
};


}}


#endif
