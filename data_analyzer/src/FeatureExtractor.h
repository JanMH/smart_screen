#ifndef SMART_SCREEN_FEATUREEXTRACTOR_H
#define SMART_SCREEN_FEATUREEXTRACTOR_H

#include <complex>
#include <cmath>
#include "EventFeatures.h"
#include "FastFourierTransformCalculator.h"
#include "Algorithms.h"

/**
 * @brief  This class extracts the feature vector out of a raw event, based on a classification configuration.
 */
class FeatureExtractor {
public:
    typedef EventFeatures::FeatureType FeatureType;

    template<typename DataPointType> EventFeatures extractFeatures(const Event<DataPointType> &event);

    void setConfig(ClassificationConfig config);

private:

    template<typename DataPointType> void
    calcFFTs(const Event<DataPointType> &event);

    template<typename DataPointType> void
    extractRms(const Event<DataPointType> &event, std::vector<FeatureType> &feature_vec);


    template<typename DataPointType> void
    extractHarmonics(const Event<DataPointType> &event, std::vector<FeatureExtractor::FeatureType> &feature_vec);

    template<typename DataPointType> void
    extractPhaseShift(const Event<DataPointType> &event, std::vector<FeatureExtractor::FeatureType> &feature_vec);

    float calcPhaseShift(const std::vector<kiss_fft_cpx> &amps, const std::vector<kiss_fft_cpx> &volts,
                         unsigned long base_freq_pos);

    unsigned long calcBaseFrequencyPos(const PowerMetaData &meta_data, unsigned long number_of_data_points_in_fft);


private:
    ClassificationConfig classification_config;
    FastFourierTransformCalculator fft_calculator;
    std::vector<kiss_fft_cpx> fft_ampere_before;
    std::vector<kiss_fft_cpx> fft_ampere_after;
    std::vector<kiss_fft_cpx> fft_voltage_before;
    std::vector<kiss_fft_cpx> fft_voltage_after;


};

template<typename DataPointType> EventFeatures FeatureExtractor::extractFeatures(const Event<DataPointType> &event) {
    calcFFTs(event);
    std::vector<FeatureType> f_vect;
    extractPhaseShift<DataPointType>(event, f_vect);

    extractRms<DataPointType>(event, f_vect);
    extractHarmonics<DataPointType>(event, f_vect);
    return EventFeatures(event.event_meta_data, f_vect);
}


template<typename DataPointType> void FeatureExtractor::extractRms(const Event<DataPointType> &event,
                                                                   std::vector<FeatureExtractor::FeatureType> &feature_vec) {
    FeatureType sub_rms = 0;
    unsigned long data_points_per_period = event.event_meta_data.power_meta_data.dataPointsPerPeriod();
    if (event.event_meta_data.power_meta_data.data_points_stored_before_event >= data_points_per_period) {
        sub_rms = Algorithms::rootMeanSquareOfAmpere(event.before_event_begin(),
                                                     event.before_event_begin() + data_points_per_period);
    }

    long loop_end = event.event_end() - event.event_begin() - data_points_per_period;
    auto begin = event.event_begin();
    auto end = event.event_begin() + data_points_per_period;
    int period_count = 0;
    for (long count = 0; count < loop_end; count += data_points_per_period) {
        FeatureType rms = Algorithms::rootMeanSquareOfAmpere(begin, end);
        rms -= sub_rms;
        feature_vec.push_back(rms);

        begin += data_points_per_period;
        end += data_points_per_period;

        if (period_count >= classification_config.number_of_rms) break;
        ++period_count;

    }
}

inline void FeatureExtractor::setConfig(ClassificationConfig config) {
    this->classification_config = config;

}

template<typename DataPointType> void
FeatureExtractor::calcFFTs(const Event<DataPointType> &event) {
    unsigned long num_data_points = event.before_event_end() - event.before_event_begin();
    num_data_points = std::min(num_data_points, static_cast<unsigned long>(event.event_end() - event.event_begin()));

    fft_ampere_before = fft_calculator.calculateAmpereFFTWithBlackmanHarris(event.before_event_begin(),
                                                                            event.before_event_begin() +
                                                                            num_data_points);
    fft_voltage_before = fft_calculator.calculateVoltageFFTWithBlackmanHarris(event.before_event_begin(),
                                                                              event.before_event_begin() +
                                                                              num_data_points);

    fft_ampere_after = fft_calculator.calculateAmpereFFTWithBlackmanHarris(event.event_begin(),
                                                                           event.event_begin() + num_data_points);
    fft_voltage_after = fft_calculator.calculateVoltageFFTWithBlackmanHarris(event.event_begin(),
                                                                             event.event_begin() + num_data_points);
}


template<typename DataPointType> void FeatureExtractor::extractHarmonics(const Event<DataPointType> &event,
                                                                         std::vector<FeatureExtractor::FeatureType> &feature_vec) {

    unsigned long base_frequency_pos = calcBaseFrequencyPos(event.event_meta_data.power_meta_data, fft_ampere_before.size());
    std::vector<FeatureExtractor::FeatureType> harm_old = Algorithms::getHarmonics(fft_ampere_before,
                                                                                   base_frequency_pos,
                                                                                   classification_config.number_of_harmonics,
                                                                                   classification_config.harmonics_search_radius);

    std::vector<FeatureExtractor::FeatureType> harm_new = Algorithms::getHarmonics(fft_ampere_after,
                                                                                   base_frequency_pos,
                                                                                   classification_config.number_of_harmonics,
                                                                                   classification_config.harmonics_search_radius);
    auto old_iter = harm_old.begin();
    for (auto &harmonic: harm_new) {
        feature_vec.push_back(std::abs(harmonic) - std::abs(*old_iter));
        ++old_iter;
    }
}

template<typename DataPointType> void FeatureExtractor::extractPhaseShift(const Event<DataPointType> &event,
                                                                          std::vector<FeatureExtractor::FeatureType> &feature_vec) {

    unsigned long base_frequency_pos = calcBaseFrequencyPos(event.event_meta_data.power_meta_data, fft_ampere_before.size());

    float phase_shift_before = calcPhaseShift(fft_ampere_before, fft_voltage_before, base_frequency_pos);

    float phase_shift = calcPhaseShift(fft_ampere_after, fft_voltage_after, base_frequency_pos);

    float phase_shift_difference = phase_shift - phase_shift_before;
    feature_vec.push_back(phase_shift_difference);


}


inline float FeatureExtractor::calcPhaseShift(const std::vector<kiss_fft_cpx> &amps, const std::vector<kiss_fft_cpx> &volts,
                                       unsigned long base_freq_pos) {

    std::complex<float> amps_cpx(amps[base_freq_pos].r, amps[base_freq_pos].i);
    std::complex<float> volts_cpx(volts[base_freq_pos].r, volts[base_freq_pos].i);

    float angle_amps = std::arg(amps_cpx);
    float angle_volts = std::arg(volts_cpx);
    float phase_tmp = (angle_amps - angle_volts) * -1;
    if (phase_tmp > M_PI) {
        return 2 * M_PI - phase_tmp;
    } else if (phase_tmp < -M_PI) {
        return 2 * M_PI + phase_tmp;
    }
    return phase_tmp;
}

inline unsigned long
FeatureExtractor::calcBaseFrequencyPos(const PowerMetaData &meta_data, unsigned long number_of_data_points_in_fft) {
    float result = number_of_data_points_in_fft;
    result /= meta_data.sample_rate;
    result *= meta_data.frequency;
    return static_cast<unsigned long>(roundf(result));
}


#endif //SMART_SCREEN_FEATUREEXTRACTOR_H
