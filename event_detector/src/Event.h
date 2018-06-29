#ifndef SMART_SCREEN_EVENT_H
#define SMART_SCREEN_EVENT_H

#include <vector>
#include "EventMetaData.h"
#include "DefaultDataPoint.h"
#include "BluedDataPoint.h"
#include <boost/serialization/vector.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

/**
 * @brief This class stores a detected event alongside it's meta data.
 * @tparam DataPointType
 */
template<typename DataPointType> class Event {
public:
    std::vector<DataPointType> event_data;
    EventMetaData event_meta_data;
    typedef float DatumType;

    /**
     * Creates an event based on raw data.
     * @tparam IteratorType The type of iterator passed. This will usually be deducted by the compiler.
     * @param meta_data The meta data, usually created by the ProgramConfiguration class
     * @param data_begin An iterator to the beginning of the class
     * @param data_end An iterator pointing past the last data point
     * @param event_id The id of the event. If it is unknown yet it can be ignored
     * @return A new Event object
     */
    template<typename IteratorType>
    static Event<DataPointType> createEvent(EventMetaData meta_data, IteratorType data_begin, IteratorType data_end,
                                            boost::optional<EventMetaData::EventIdType> event_id = boost::none);

    /**
     *
     * @return An iterator pointing to the first data point of data that was recorded before the event happened
     */
    constexpr typename std::vector<DataPointType>::const_iterator before_event_begin() const {
        return event_data.begin();
    }

    /**
     *
     * @return An iterator pointing past the last data point that was recroded before the event happened
     */
    constexpr typename std::vector<DataPointType>::const_iterator before_event_end() const {
        return event_begin();

    }

    /**
     *
     * @return An iterator that points to the first data point recorded during the switch on event
     */
    constexpr typename std::vector<DataPointType>::const_iterator event_begin() const {
        return event_data.begin() + event_meta_data.power_meta_data.data_points_stored_before_event;
    }

    /**
     *
     * @return An iterator pointing past the last recorded data point
     */
    constexpr typename std::vector<DataPointType>::const_iterator event_end() const {
        return event_data.end();
    }

private:

    template<typename IteratorType> static  std::vector<DataPointType>
    eventDataToVector(IteratorType begin, const IteratorType end);


};


template<typename DataPointType>
template<typename IteratorType> Event<DataPointType>
Event<DataPointType>::createEvent(EventMetaData meta_data, IteratorType data_begin, IteratorType data_end,
                   boost::optional<EventMetaData::EventIdType> event_id) {
    Event<DataPointType> event;

    event.event_data = eventDataToVector<IteratorType>(data_begin, data_end);
    event.event_meta_data = meta_data;
    event.event_meta_data.event_id = event_id;
    return event;
}

template<typename DataPointType>
template<typename IteratorType> std::vector<DataPointType>
Event<DataPointType>::eventDataToVector(IteratorType begin, const IteratorType end) {
    return std::vector<DataPointType>(begin, end);
}

namespace boost {
    namespace serialization {

        template<class Archive> void serialize(Archive &ar, DefaultDataPoint &data_point, const unsigned int version) {
            ar & data_point.volts;
            ar & data_point.amps;
        }

        template<class Archive> void serialize(Archive &ar, EventMetaData &meta_data, const unsigned int version) {

            ar & meta_data.event_id;
            ar & meta_data.event_time;
            ar & meta_data.label;
            ar & meta_data.power_meta_data;

        }

        template<class Archive> void serialize(Archive &ar, PowerMetaData &meta_data, const unsigned int version) {
            ar & meta_data.sample_rate; /**< The sample rate of the current. 12000 for BLUED, 16000 for UK-DALE */
            ar & meta_data.frequency; /**< The frequency in Hz of the current. */
            ar & meta_data.voltage; /**< The voltage of the current. In the European  Union 220 V is the standard */

            ar &
            meta_data.max_data_points_in_queue; /**< The number of samples we store unitl the writing thread is blocked.*/
            ar &
            meta_data.data_points_stored_of_event; /**< The number of samples we store until the writing thread is blocked.*/
            ar & meta_data.data_points_stored_before_event;

        }

        template<class Archive> void
        serialize(Archive &ar, Event<DefaultDataPoint> &event, const unsigned int version) {
            ar & event.event_data;
            ar & event.event_meta_data;
        }

        template<class Archive> void serialize(Archive &ar, Event<BluedDataPoint> &event, const unsigned int version) {
            ar & event.event_data;
            ar & event.event_meta_data;
        }

        template<class Archive> void serialize(Archive &ar, BluedDataPoint &dp, const unsigned int version) {
            ar & dp.x_value;
            ar & dp.current_a;
            ar & dp.current_b;
            ar & dp.voltage_a;
        }

    } // namespace serialization
} // namespace boost


#endif //SMART_SCREEN_EVENT_H
