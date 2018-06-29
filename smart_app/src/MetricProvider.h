
#ifndef SMART_SCREEN_SMARTMETERWORKER_H
#define SMART_SCREEN_SMARTMETERWORKER_H

#include <Algorithms.h>
#include <boost/signals2/signal.hpp>
#include "DataStreamWorker.h"

template<typename DataPointType>
class MetricProvider : public DataStreamWorker<DataPointType> {
public:
    virtual typename DataStreamWorker<DataPointType>::NextReadInfos
    workOnDataPoints(typename DataStreamTypes<DataPointType>::DataStreamIterator begin,
                     typename DataStreamTypes<DataPointType>::DataStreamIterator end,
                     DynamicStreamMetaData::DataPointIdType data_point_id) override;

public: // Attributes


private:
    std::size_t numberOfDataPointsFetched();

    typename DataStreamWorker<DataPointType>::NextReadInfos
    nextPeriod(DynamicStreamMetaData::DataPointIdType data_point_id);

    void calculateSteadyStateData(typename DataStreamTypes<DataPointType>::DataStreamIterator begin,
                                  typename DataStreamTypes<DataPointType>::DataStreamIterator end,
                                  DynamicStreamMetaData::DataPointIdType data_point_id);

    void resetInfos(const DynamicStreamMetaData::DataPointIdType data_point_id);

    bool shouldPublishData();

    void publishData();

private: //attributes
    std::size_t total_seconds_of_data_accumulated;
    std::size_t current_seconds_of_data_accumulated;


};

template<typename DataPointType>
typename DataStreamWorker<DataPointType>::NextReadInfos
MetricProvider<DataPointType>::workOnDataPoints(typename DataStreamTypes<DataPointType>::DataStreamIterator begin,
                                                typename DataStreamTypes<DataPointType>::DataStreamIterator end,
                                                DynamicStreamMetaData::DataPointIdType data_point_id) {
    if (begin != end) {
        program_log->debug("MetricProvider: Requested data point {},  got {} data points", data_point_id, end - begin);

        calculateSteadyStateData(begin, end, data_point_id);
        if (shouldPublishData()) {
            publishData();
            resetInfos(data_point_id);
        }

    }
    return nextPeriod(data_point_id);
}

template<typename DataPointType>
typename DataStreamWorker<DataPointType>::NextReadInfos
MetricProvider<DataPointType>::nextPeriod(DynamicStreamMetaData::DataPointIdType data_point_id) {
    auto number_of_data_points = numberOfDataPointsFetched();
    auto next_read_location = data_point_id + number_of_data_points;
    std::size_t keep_available = 0;
    return typename DataStreamWorker<DataPointType>::NextReadInfos(keep_available, number_of_data_points,
                                                                   next_read_location);
}

template<typename DataPointType> void
MetricProvider<DataPointType>::calculateSteadyStateData(
        typename DataStreamTypes<DataPointType>::DataStreamIterator begin,
        typename DataStreamTypes<DataPointType>::DataStreamIterator end,
        DynamicStreamMetaData::DataPointIdType data_point_id) {
    assert(end - begin == this->stream_meta_data->getFixedPowerMetaData().sample_rate);

}

template<typename DataPointType>
std::size_t MetricProvider<DataPointType>::numberOfDataPointsFetched() {
    return this->stream_meta_data->getFixedPowerMetaData().sample_rate;
}

template<typename DataPointType>
void MetricProvider<DataPointType>::resetInfos(const DynamicStreamMetaData::DataPointIdType data_point_id) {


}

template<typename DataPointType>
bool MetricProvider<DataPointType>::shouldPublishData() {
    return current_seconds_of_data_accumulated >= total_seconds_of_data_accumulated;
}

template<typename DataPointType>
void MetricProvider<DataPointType>::publishData() {

}


#endif //SMART_SCREEN_SMARTMETERWORKER_H
