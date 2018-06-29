#ifndef SMART_SCREEN_EVENT_DETECTOR_H
#define SMART_SCREEN_EVENT_DETECTOR_H

#include "DefaultDataPoint.h"
#include "Algorithms.h"
#include "DataStreamWorker.h"
#include "Event.h"
#include "Logger.h"

#include <boost/signals2.hpp>

/**
 * @brief This is a data stream worker that detects an event.
 * If you only wish to use this class the two relevant parts are the constructor and the event_detected signal.
 * @tparam DataPointType
 */
template<typename DataPointType>
class EventDetector : public DataStreamWorker<DataPointType> {
public:
    EventDetector<DataPointType>(float detection_threshold = 0.2,
                                 DynamicStreamMetaData::DataPointIdType detection_chunk_size = 800) : threshold(
            detection_threshold), compare_after(detection_chunk_size) {
    }


    virtual typename DataStreamWorker<DataPointType>::NextReadInfos
    workOnDataPoints(typename DataStreamTypes<DataPointType>::DataStreamIterator begin,
                     typename DataStreamTypes<DataPointType>::DataStreamIterator end,
                     DynamicStreamMetaData::DataPointIdType data_point_id) {
        auto data_point_id_casted = static_cast<std::size_t > (data_point_id);
        if (begin == end) {
            return getFirstDetectionPeriod(data_point_id);
        }

        if (this->detected_an_event && this->requested_data_point_id == data_point_id) {
            this->detected_an_event = false;
            this->event_detected(createEvent(begin, end, data_point_id));
            return getDataAfterEvent(data_point_id);
        }

        if (detectEvent(begin, end)) {
            // ask the DataStreamDistributor for all the required data
            detected_an_event = true;
            return getEventData(data_point_id);
        } else {
            return getNextDetectionPeriod(data_point_id);
        }
    }

    void setThreshold(float threshold) {
        this->threshold = threshold;
    }

    void setCompareAfter(const DynamicStreamMetaData::DataPointIdType &chunk_size) {
        EventDetector::compare_after = chunk_size;
    }

private: // methods



    typename DataStreamWorker<DataPointType>::NextReadInfos
    getEventData(DynamicStreamMetaData::DataPointIdType current_data_point_id) {

        auto total_number_of_data_points =
                this->stream_meta_data->getFixedPowerMetaData().data_points_stored_before_event +
                this->stream_meta_data->getFixedPowerMetaData().data_points_stored_of_event;
        requested_data_point_id =
                current_data_point_id - this->stream_meta_data->getFixedPowerMetaData().data_points_stored_before_event + dataPointsPerPeriod();
        auto keep_at_least = 0;

        return typename DataStreamWorker<DataPointType>::NextReadInfos(keep_at_least, total_number_of_data_points,
                                                                       requested_data_point_id);
    }

    typename DataStreamWorker<DataPointType>::NextReadInfos
    getDataAfterEvent(DynamicStreamMetaData::DataPointIdType current_data_point_id) {

        auto total_number_of_data_points = minDataPointsNeeded();
        requested_data_point_id = current_data_point_id +
                                  this->stream_meta_data->getFixedPowerMetaData().data_points_stored_before_event +
                                  this->stream_meta_data->getFixedPowerMetaData().data_points_stored_of_event;
        auto keep_at_least = this->stream_meta_data->getFixedPowerMetaData().data_points_stored_before_event;

        return typename DataStreamWorker<DataPointType>::NextReadInfos(keep_at_least, total_number_of_data_points,
                                                                       requested_data_point_id);
    }

    typename DataStreamWorker<DataPointType>::NextReadInfos
    getFirstDetectionPeriod(DynamicStreamMetaData::DataPointIdType current_data_point_id) {

        auto total_number_of_data_points = minDataPointsNeeded();
        auto keep_at_least = this->stream_meta_data->getFixedPowerMetaData().data_points_stored_before_event;
        requested_data_point_id = current_data_point_id + keep_at_least;

        return typename DataStreamWorker<DataPointType>::NextReadInfos(keep_at_least, total_number_of_data_points,
                                                                       requested_data_point_id);
    }

    typename DataStreamWorker<DataPointType>::NextReadInfos
    getNextDetectionPeriod(DynamicStreamMetaData::DataPointIdType current_data_point_id) {

        auto total_number_of_data_points = minDataPointsNeeded();
        requested_data_point_id = current_data_point_id + total_number_of_data_points;
        auto keep_at_least = this->stream_meta_data->getFixedPowerMetaData().data_points_stored_before_event;

        return typename DataStreamWorker<DataPointType>::NextReadInfos(keep_at_least, total_number_of_data_points,
                                                                       requested_data_point_id);
    }

    std::size_t minDataPointsNeeded() {
        DynamicStreamMetaData::DataPointIdType data_points_needed_for_new_median =
                this->number_of_rms * dataPointsPerPeriod();
        return static_cast<size_t >( std::max(data_points_needed_for_new_median, this->compare_after) + dataPointsPerPeriod());

    }

    void initializePreviousRMS(typename DataStreamTypes<DataPointType>::DataStreamIterator begin,
                               typename DataStreamTypes<DataPointType>::DataStreamIterator end) {
        auto data_points_available = end-begin;
        assert(end-begin > this->dataPointsPerPeriod() * this->number_of_rms);

        for(int i = 0; i< number_of_rms; ++i) {
            this->previous_rms.push_back(
                    Algorithms::rootMeanSquareOfAmpere(begin + (i*dataPointsPerPeriod()), begin + (i+1)*dataPointsPerPeriod())
            );
        }

    }

    unsigned long dataPointsPerPeriod() {
        return this->stream_meta_data->getFixedPowerMetaData().dataPointsPerPeriod();
    }

    bool detectEvent(typename DataStreamTypes<DataPointType>::DataStreamIterator begin,
                     typename DataStreamTypes<DataPointType>::DataStreamIterator end) {
        assert(end - begin >= dataPointsPerPeriod());
        assert(end - begin >= this->compare_after);


        previous_rms.push_back(Algorithms::rootMeanSquareOfAmpere(begin, end));

        if (previous_rms.size() > number_of_rms) { // we have more than enough elements in our buffer. Let's move on.
            previous_rms.pop_front();
        } else if (previous_rms.size() < number_of_rms) {
            initializePreviousRMS(begin, end);
            return false;
        }

        float current_rms = Algorithms::rootMeanSquareOfAmpere(begin, end);
        auto median = this->getMedianPrevRMS();

        if (current_rms - threshold > median) {
            this->previous_rms.clear();
            return true;
        }
        return false;
    }

    std::shared_ptr<Event<DataPointType>> createEvent(typename DataStreamTypes<DataPointType>::DataStreamIterator begin,
                                                      typename DataStreamTypes<DataPointType>::DataStreamIterator end,
                                                      DynamicStreamMetaData::DataPointIdType data_point_id) {
        auto total_event_length = this->stream_meta_data->getFixedPowerMetaData().data_points_stored_before_event
                                  + this->stream_meta_data->getFixedPowerMetaData().data_points_stored_of_event;
        if (end - begin < total_event_length) {
            program_log->info("Not enough data points to store the event. {} were provided and {} are needed.",
                              end - begin, total_event_length);
            throw std::exception();
        }
        auto event_start =
                data_point_id + this->stream_meta_data->getFixedPowerMetaData().data_points_stored_before_event;
        auto event_time = this->stream_meta_data->getDataPointTime(event_start);

        EventMetaData meta_data(event_time, this->stream_meta_data->getFixedPowerMetaData());
        auto event = std::shared_ptr<Event<DataPointType>>(new Event<DataPointType>);
        *event = Event<DataPointType>::createEvent(meta_data, begin, end);
        return event;

    }

    float getMedianPrevRMS() {
        if (previous_rms.empty()) {
            return 0.0;
        }
        auto working_copy = previous_rms;
        std::nth_element(working_copy.begin(), working_copy.begin() + working_copy.size() / 2, working_copy.end());
        return working_copy[working_copy.size() / 2];
    }


public:
    boost::signals2::signal<void(std::shared_ptr<Event<DataPointType>>)> event_detected;

private:
    DynamicStreamMetaData::DataPointIdType requested_data_point_id;
    float threshold;

    DynamicStreamMetaData::DataPointIdType compare_after;

    bool detected_an_event = false;

    std::deque<float> previous_rms;
    std::size_t number_of_rms = 4;

};


#endif //SMART_SCREEN_EVENT_DETECTOR_H
