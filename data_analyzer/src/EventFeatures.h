#ifndef SMART_SCREEN_EVENTFEATURES_H
#define SMART_SCREEN_EVENTFEATURES_H

#include <vector>
#include "EventMetaData.h"
#include "Event.h"
#include "Algorithms.h"
#include "ClassificationConfig.h"

/**
 * @brief This struct contains a raw unnormalized feature vector alongside the meta data of an event.
 */
struct EventFeatures {

    typedef float FeatureType;

    EventMetaData event_meta_data;

    std::vector<FeatureType> feature_vector;

    EventFeatures() = default;


    EventFeatures(EventMetaData meta_data, const std::vector<FeatureType>& f_vect) :event_meta_data(meta_data),
                                                                              feature_vector(f_vect) {}


};




#endif //SMART_SCREEN_EVENTFEATURES_H
