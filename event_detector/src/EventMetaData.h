#ifndef EVENTMETADATA_H
#define EVENTMETADATA_H

#include <boost/date_time.hpp>
#include <boost/optional.hpp>
#include "DynamicStreamMetaData.h"

/**
 * @brief this structure contains meta data of detected and classified events.
 */
struct EventMetaData {
    enum Definitions {
        EventUnknown = 0
    };

    typedef DynamicStreamMetaData::TimeType TimeType; ///< A type that is capable of storing time points.

    typedef DynamicStreamMetaData::MSDurationType MSDurationType; ///< A type that encodes time durations in milliseconds
    typedef DynamicStreamMetaData::USDurationType USDurationType;  ///< A type that encodes time durations in microseconds
    typedef unsigned long EventIdType; ///< A numeric type that can contain event ids
    typedef double LabelType; ///< A numeric type that encodes label ids


    TimeType event_time; ///< the time, at which the event happened. Note that this time correlates with Event<DataPointType>::event_begin() not the start of the event data vector
    boost::optional<EventIdType> event_id; ///< An event id. If the event does not have an id yet, this equals boost::none
    boost::optional<LabelType> label; ///< The label (device id) of the event.
    float classification_certainty = 0.0f; ///< The classification certainty of the event. (calculated from the number of neighbors with the same id divided by the complete number of neighbors)
    PowerMetaData power_meta_data; ///< The power meta data that was used during the recording of the event

    EventMetaData() {}


    EventMetaData(TimeType time, PowerMetaData meta_data) : event_time(time), power_meta_data(meta_data) {}

};

#endif // EVENTMETADATA_H
