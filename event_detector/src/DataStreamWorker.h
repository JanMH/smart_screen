
#ifndef SMART_SCREEN_DATASTREAMWORKER_H
#define SMART_SCREEN_DATASTREAMWORKER_H


#include <cstddef>
#include <DynamicStreamMetaData.h>
#include "DataStreamTypes.h"

/**
 * @brief This class performs calculations on the immutable data stream
 */
template<typename DataPointType>
class DataStreamWorker {
public:
    struct NextReadInfos {

        NextReadInfos(std::size_t _keep_number_of_data_points_available = 0,
                      std::size_t _read_number_of_data_points = 0,
                      DynamicStreamMetaData::DataPointIdType _next_read_location = 0)
                : keep_number_of_data_points_available(_keep_number_of_data_points_available),
                  read_number_of_data_points(_read_number_of_data_points),
                  next_read_location(_next_read_location) {}

        std::size_t keep_number_of_data_points_available = 0; ///< Number of data points that should be kept in the buffer after the next read
        std::size_t read_number_of_data_points = 0; ///< number of data points that should be read and passed to workOnDataPoints in the next read
        DynamicStreamMetaData::DataPointIdType next_read_location = 0; ///< position of the next read. Can be before the current read location but not before current_location - keep_number_of_data_points_available
    };


    /**
     * @brief This functions performs the actual calculations on the data stream.
     *
     * @param begin
     * @param end
     * @return Returns information on the read
     */
    virtual NextReadInfos
    workOnDataPoints(typename DataStreamTypes<DataPointType>::DataStreamIterator begin,
                     typename DataStreamTypes<DataPointType>::DataStreamIterator end,
                     DynamicStreamMetaData::DataPointIdType data_point_id) = 0;

    void setStreamMetaData(std::shared_ptr<DynamicStreamMetaData> meta_data) {
        this->stream_meta_data = std::move(meta_data);
    }


protected:
    std::shared_ptr<DynamicStreamMetaData> stream_meta_data;
};


#endif //SMART_SCREEN_DATASTREAMWORKER_H
