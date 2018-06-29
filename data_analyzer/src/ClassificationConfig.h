#ifndef SMART_SCREEN_CLASSIFICATIONCONFIG_H
#define SMART_SCREEN_CLASSIFICATIONCONFIG_H


/**
 * @brief This enum encodes the different types of feature scaling.
 */
enum class NormalizationMode {
    Rescale,
    Standardize
};


/**
 * @brief This struct contains configurations on how the DataClassifier should classify events.
 */
struct ClassificationConfig {
    int number_of_rms = 20; ///< The number of periods from which the root mean square will be extracted.
    NormalizationMode normalization_mode = NormalizationMode::Standardize; ///< The algorithm with which the feature matrix will be normalized. Refer to [the wikipedia article on feature scaling](https://en.wikipedia.org/wiki/Feature_scaling) for moore information.
    unsigned long number_of_harmonics = 10; ///< The number of harmonics that will be extracted.
    unsigned long harmonics_search_radius = 5; ///< @see Algorithms::getHarmonics for more infos abuot the nature of this attribute


};


#endif //SMART_SCREEN_CLASSIFICATIONCONFIG_H
