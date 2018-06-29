#ifndef _SMART_SCREEN_DYNAMICSTREAMMETADATA_H
#define _SMART_SCREEN_DYNAMICSTREAMMETADATA_H

#include <mutex>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/date_time.hpp>
#include "PowerMetaData.h"

/**
 * @brief This class synchronizes the time between the insertion of data points into the queue and reading them from the queue.
 *
 * When reading data from a data source or prerecorded data set, the time between reading and detecting an event might be several seconds. To make sure the evnt detection time is set to the time of reading, this class gives each data point an id. The writing set can then set the time for select data points. When the reader thread needs to know the time of a specific data point, it queries this class which approximates the time based on the last syced data point.
 */
class DynamicStreamMetaData {
public:
    typedef boost::multiprecision::cpp_int DataPointIdType;
    typedef boost::posix_time::ptime TimeType;
    typedef boost::posix_time::milliseconds MSDurationType;
    typedef boost::posix_time::microseconds USDurationType;


    DynamicStreamMetaData(DataPointIdType starting_package_id = 0) : synced_package_id(
            starting_package_id){}
    /**
     * Sets the power meta data containing frequency and other infos
     * @param fixed_meta_data
     */
    void setFixedPowerMetaData(PowerMetaData fixed_meta_data);
    PowerMetaData getFixedPowerMetaData() const;

    /**
     * Notifies the dynamic stream meta data of the time stamp for a data point.
     * @param data_point_number
     * @param time
     */
    void syncTimePoint(DataPointIdType data_point_number, TimeType time);

    /**
     * Gets the time for a data point based on the last synced timestamp data point pair and the sampling rate.
     * @param data_point_number
     * @return
     */
    TimeType getDataPointTime(DataPointIdType data_point_number);


private:
    std::mutex sync_mutex;
    DataPointIdType synced_package_id;
    DynamicStreamMetaData::TimeType packet_time;
    PowerMetaData power_meta_data;

};


#endif //SMART_SCREEN_DYNAMICSTREAMMETADATA_H
