#ifndef SMART_SCREEN_DATAVISUALIZER_H
#define SMART_SCREEN_DATAVISUALIZER_H

#include <EventStorage.h>
#include <string>
#include <fstream>
#include <FeatureExtractor.h>


template<typename DataPointType> class EventVisualizer {
public:
    void setEventStorageDirectory(const std::string &dir);

    void printEvent(unsigned long event_id, std::ostream &out_stream = std::cout);

    void printFeatureVector(unsigned long event_id, std::ostream &out_stream = std::cout);

    void printFFTOfEvent(unsigned long event_id, std::ostream &out_stream = std::cout);

    void printFFTOfEventLimitDataPoints(unsigned long event_id, std::ostream &out_stream = std::cout);

    void printFFTBeforeEvent(unsigned long event_id, std::ostream &out_stream = std::cout);

    void printBlackmanHarris(unsigned long number_of_elements, std::ostream &out_stream = std::cout);

    //void printWindowedFFT(unsigned long event_id, std::ostream &out_stream = std::cout);


private:
    FeatureExtractor feature_extractor;
    EventStorage<DataPointType> storage;

};


template<typename DataPointType> void EventVisualizer<DataPointType>::setEventStorageDirectory(const std::string &dir) {
    storage.event_directory = dir;
}

template<typename DataPointType> void
EventVisualizer<DataPointType>::printEvent(unsigned long event_id, std::ostream &out_stream) {
    auto e = storage.loadEvent(event_id);
    for (auto &data_point: e.event_data) {
        out_stream << data_point.voltage() << "," << data_point.ampere() << "\n";
    }
}


template<typename DataPointType> void
EventVisualizer<DataPointType>::printFeatureVector(unsigned long event_id, std::ostream &out_stream) {
    auto e = storage.loadEvent(event_id);
    auto features = feature_extractor.extractFeatures(e);
    for (auto &feature: features.feature_vector) {
        out_stream << feature << "\n";
    }

}

template<typename DataPointType> void
EventVisualizer<DataPointType>::printFFTOfEvent(unsigned long event_id, std::ostream &out_stream) {
    auto e = storage.loadEvent(event_id);
    FastFourierTransformCalculator fftc;
    auto fft = fftc.calculateAmpereFFTWithBlackmanHarris(e.event_begin(), e.event_end());
    for (const auto fft_piont:fft) {
        out_stream << fft_piont.r << "\n";
    }
}

template<typename DataPointType> void
EventVisualizer<DataPointType>::printFFTOfEventLimitDataPoints(unsigned long event_id, std::ostream &out_stream) {
    auto e = storage.loadEvent(event_id);

    auto num_data_points = e.before_event_end() - e.before_event_begin();
    num_data_points = std::min(num_data_points, e.event_end() - e.event_begin());

    FastFourierTransformCalculator fftc;
    auto fft = fftc.calculateAmpereFFTWithBlackmanHarris(e.event_begin(), e.event_begin() + num_data_points);
    for (const auto fft_piont:fft) {
        out_stream << fft_piont.r << "\n";
    }
}


template<typename DataPointType> void EventVisualizer<DataPointType>::printFFTBeforeEvent(unsigned long event_id, std::ostream &out_stream) {
    auto e = storage.loadEvent(event_id);
    FastFourierTransformCalculator fftc;
    auto fft = fftc.calculateAmpereFFTWithBlackmanHarris(e.before_event_begin(), e.before_event_end());
    for (const auto fft_piont:fft) {
        out_stream << fft_piont.r << "\n";
    }
}

template<typename DataPointType> void EventVisualizer<DataPointType>::printBlackmanHarris(unsigned long number_of_elements, std::ostream &out_stream) {
    FastFourierTransformCalculator fftc;
    for (const auto bmh:fftc.getBlackmanHarrisBuffer(number_of_elements)) {
        out_stream << bmh<< "\n";
    }

}


#endif //SMART_SCREEN_DATAVISUALIZER_H
