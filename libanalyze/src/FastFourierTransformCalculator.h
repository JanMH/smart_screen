#ifndef SMART_SCREEN_FOURIERTRANSFORMCALCULATOR_H
#define SMART_SCREEN_FOURIERTRANSFORMCALCULATOR_H

#include <kiss_fft.h>
#include <memory>
#include <vector>
#include <cmath>
#include "Utilities.h"

class FastFourierTransformCalculator {
public:
    typedef float DataPointType;

    FastFourierTransformCalculator() {

    }

    FastFourierTransformCalculator(unsigned long _max_data_size) {
        this->setMaxDataSetSize(_max_data_size);
    }

    void setMaxDataSetSize(unsigned long _max_data_size) {
        max_data_size = _max_data_size;
        kiss_fft_buffer.resize(max_data_size);
        size_t lenmem = 1024 + (_max_data_size) * sizeof(kiss_fft_cpx);
        cfg_buffer.resize(lenmem);
    }


    template<typename IteratorType> std::vector<kiss_fft_cpx> calculateAmpereFFT(IteratorType begin, IteratorType end);

    template<typename IteratorType> std::vector<kiss_fft_cpx> calculateVoltageFFT(IteratorType begin, IteratorType end);
    template<typename IteratorType> std::vector<kiss_fft_cpx> calculateFFT(IteratorType begin, IteratorType end);

    template<typename IteratorType> std::vector<kiss_fft_cpx> calculateAmpereFFTWithBlackmanHarris(IteratorType begin, IteratorType end);
    template<typename IteratorType> std::vector<kiss_fft_cpx> calculateVoltageFFTWithBlackmanHarris(IteratorType begin, IteratorType end);

    template<typename IteratorType> std::vector<kiss_fft_cpx> calculateFFTWithBlackmanHarris(IteratorType begin, IteratorType end);


    kiss_fft_cfg initKissFFT(const unsigned long N);
    template<typename IteratorType> void fillKissFFTBuffer(IteratorType begin, IteratorType end);
    void multiplyKissFFTBufferWithBMH(const unsigned long N);
    const std::vector<DataPointType>& getBlackmanHarrisBuffer(const unsigned long N);

    std::vector<kiss_fft_cpx> kiss_fft_buffer;
    std::vector<DataPointType> blackman_harris_buffer;


    unsigned long max_data_size = 0;
    std::vector<char> cfg_buffer;
};

 template<typename IteratorType> std::vector<kiss_fft_cpx>
FastFourierTransformCalculator::calculateAmpereFFT(IteratorType begin, IteratorType end) {
    return calculateFFT(makeAmpereIterator(begin), makeAmpereIterator(end));
}

template<typename IteratorType> std::vector<kiss_fft_cpx>
FastFourierTransformCalculator::calculateVoltageFFT(IteratorType begin, IteratorType end) {
    return calculateFFT(makeVoltageIterator(begin), makeVoltageIterator(end));
}

template<typename IteratorType> std::vector<kiss_fft_cpx>
FastFourierTransformCalculator::calculateFFT(IteratorType begin, IteratorType end) {
    unsigned long points_to_compute = end - begin;
    auto cfg = initKissFFT(points_to_compute);
    fillKissFFTBuffer(begin,end);
    std::vector<kiss_fft_cpx> kiss_fft_result(points_to_compute);

    kiss_fft(cfg, kiss_fft_buffer.data(), kiss_fft_result.data());
    return kiss_fft_result;
}

inline const std::vector<FastFourierTransformCalculator::DataPointType> &
FastFourierTransformCalculator::getBlackmanHarrisBuffer(const unsigned long N) {
    if(blackman_harris_buffer.size() == N ) {
        return this->blackman_harris_buffer;
    }
    this->blackman_harris_buffer.resize(N);

    const float a0      = 0.35875f;
    const float a1      = 0.48829f;
    const float a2      = 0.14128f;
    const float a3      = 0.01168f;

    unsigned int idx    = 0;
    while( idx < N )
    {
        blackman_harris_buffer[idx]   = a0 - (a1 * cosf( (2.0f * M_PI * idx) / (N - 1) )) + (a2 * cosf( (4.0f * M_PI * idx) / (N - 1) )) - (a3 * cosf( (6.0f * M_PI * idx) / (N - 1) ));
        idx++;
    }
    return this->blackman_harris_buffer;
}

template<typename IteratorType> std::vector<kiss_fft_cpx>
FastFourierTransformCalculator::calculateFFTWithBlackmanHarris(IteratorType begin, IteratorType end) {
    unsigned long points_to_compute = end - begin;
    auto cfg = initKissFFT(points_to_compute);
    fillKissFFTBuffer(begin,end);
    std::vector<kiss_fft_cpx> kiss_fft_result(points_to_compute);
    multiplyKissFFTBufferWithBMH(points_to_compute);

    kiss_fft(cfg, kiss_fft_buffer.data(), kiss_fft_result.data());
    return kiss_fft_result;
}

inline kiss_fft_cfg FastFourierTransformCalculator::initKissFFT(const unsigned long N) {
    if (N > max_data_size) {
        this->setMaxDataSetSize(N);
    }


    size_t lenmem = this->cfg_buffer.size();
    return kiss_fft_alloc(static_cast<int>(N), 0,
                                      reinterpret_cast<void *>(cfg_buffer.data()), &lenmem);
}

template<typename IteratorType> void
FastFourierTransformCalculator::fillKissFFTBuffer(IteratorType begin, IteratorType end) {
    int counter = 0;
    while (begin != end && counter < kiss_fft_buffer.size()) {
        kiss_fft_buffer[counter].i = 0;
        kiss_fft_buffer[counter].r = *begin;
        ++begin;
        ++counter;
    }
}

inline void FastFourierTransformCalculator::multiplyKissFFTBufferWithBMH(const unsigned long N) {
    auto bmh_iter = this->getBlackmanHarrisBuffer(N).begin();
    for(auto& cpx: kiss_fft_buffer) {
        cpx.r = *bmh_iter * cpx.r;
        ++bmh_iter;
    }
}

template<typename IteratorType> std::vector<kiss_fft_cpx>
FastFourierTransformCalculator::calculateAmpereFFTWithBlackmanHarris(IteratorType begin, IteratorType end) {
    return calculateFFTWithBlackmanHarris(makeAmpereIterator(begin), makeAmpereIterator(end));
}

template<typename IteratorType> std::vector<kiss_fft_cpx>
FastFourierTransformCalculator::calculateVoltageFFTWithBlackmanHarris(IteratorType begin, IteratorType end) {
    return calculateFFTWithBlackmanHarris(makeVoltageIterator(begin), makeVoltageIterator(end));
}

#endif //SMART_SCREEN_FOURIERTRANSFORMCALCULATOR_H
