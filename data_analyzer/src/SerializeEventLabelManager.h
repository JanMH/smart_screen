/**
 * @file
 * This file contains functions to make the serialization of the event label manager work with boost serialize.
 */

#ifndef SMART_SCREEN_SERIALIZEEVENTLABELMANAGER_H
#define SMART_SCREEN_SERIALIZEEVENTLABELMANAGER_H

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/set.hpp>


#include "EventLabelManager.h"

namespace boost {
    namespace serialization {

        template<class Archive, typename DataPointType>
        void serializeEventLabelManager(Archive &ar, EventLabelManager<DataPointType> &label_manager,
                                        const unsigned int version) {
            ar & label_manager.labeled_events;
            ar & label_manager.unlabeled_events;
            ar & label_manager.labels;
        }

        template<class Archive> void
        serialize(Archive &ar, EventLabelManager<BluedDataPoint> &label_manager, const unsigned int version) {
            serializeEventLabelManager<Archive,BluedDataPoint>(ar, label_manager, version);
        }

        template<class Archive> void
        serialize(Archive &ar, EventLabelManager<DefaultDataPoint> &label_manager, const unsigned int version) {
            serializeEventLabelManager<Archive,DefaultDataPoint>(ar, label_manager, version);
        }

        template<class Archive> void
        serialize(Archive &ar, LabelTimePair &label_time_pair, const unsigned int version) {
            ar & label_time_pair.label;
            ar & label_time_pair.time;
        }

        template<class Archive> void serialize(Archive &ar, EventFeatures &features, const unsigned int version) {
            ar & features.event_meta_data;
            ar & features.feature_vector;
        }
    }
}

#endif //SMART_SCREEN_SERIALIZEEVENTLABELMANAGER_H
