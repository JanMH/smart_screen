#include "DynamicStreamMetaData.h"

void DynamicStreamMetaData::syncTimePoint(DynamicStreamMetaData::DataPointIdType data_point_number,
                                          DynamicStreamMetaData::TimeType time) {
    std::lock_guard<std::mutex> time_lock(sync_mutex);
    this->synced_package_id = data_point_number;
    this->packet_time = time;
}

DynamicStreamMetaData::TimeType
DynamicStreamMetaData::getDataPointTime(DynamicStreamMetaData::DataPointIdType data_point_number){
    std::lock_guard<std::mutex> time_lock(sync_mutex);
    DataPointIdType data_point_difference = data_point_number - this->synced_package_id;
    data_point_difference *= DataPointIdType(1000000);
    data_point_difference /= DataPointIdType(power_meta_data.sample_rate);

    return this->packet_time + USDurationType(static_cast<int>(data_point_difference));
}

void DynamicStreamMetaData::setFixedPowerMetaData(PowerMetaData fixed_meta_data) {
    std::lock_guard<std::mutex> time_lock(sync_mutex);
    this->power_meta_data = fixed_meta_data;


}

PowerMetaData DynamicStreamMetaData::getFixedPowerMetaData() const {
    return this->power_meta_data;
}
