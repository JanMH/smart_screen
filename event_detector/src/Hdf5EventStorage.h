/// @file
#ifndef SMART_SCREEN_HDF5EVENTSTORAGE_H
#define SMART_SCREEN_HDF5EVENTSTORAGE_H

#include <unistd.h>
#include <Hdf5Mutex.h>

#include "Logger.h"
#include "Event.h"
#include "H5Cpp.h"
#include "Hdf5Serializations.h"

/**
 * A storage that can store events in hdf5 format.
 * @tparam DataPointType
 */
template<typename DataPointType>
class Hdf5EventStorage {
private: // attributes
    std::string storage_path = "events";
public:
    Hdf5EventStorage() = default;

    /**
     *
     * @param file_storage_path The folder in which the events will be stored
     */
    Hdf5EventStorage(const std::string &file_storage_path) : storage_path(file_storage_path) {}


    void storeEvent(const Event<DataPointType> &to_store);


private:
    std::string getFilePathForEvent(const EventMetaData &meta_data) const;

    static void storeMetaDataInFile(H5::H5File &file, const EventMetaData &meta_data);

};

template<typename DataPointType>
void Hdf5EventStorage<DataPointType>::storeEvent(const Event<DataPointType> &to_store) {
    std::lock_guard<std::mutex> guard(hdf5_mutex);

    H5std_string FILE_NAME(getFilePathForEvent(to_store.event_meta_data));
    auto file = H5::H5File(FILE_NAME, H5F_ACC_TRUNC);
    hdf5_serializations::serializeEvent(file, to_store.event_data.begin(), to_store.event_data.end());
    storeMetaDataInFile(file, to_store.event_meta_data);
}

template<typename DataPointType>
std::string Hdf5EventStorage<DataPointType>::getFilePathForEvent(const EventMetaData &meta_data) const {
    std::string event_id;
    if (meta_data.event_id.is_initialized()) {
        event_id = std::to_string(meta_data.event_id.get());
    } else {
        event_id = "no_id";
        program_log->error(
                "Event being stored has no id. This event will not be distinguishable. Please fix the program.");
    }

    const size_t pad_width = 7;
    event_id = pad_width < event_id.size() ? event_id :
               std::string(pad_width - event_id.size(), '0') + event_id;
    auto full_path = storage_path + "/event_" + event_id + ".hdf5";
    return full_path;
}

template<typename DataPointType>
void Hdf5EventStorage<DataPointType>::storeMetaDataInFile(H5::H5File &file, const EventMetaData &meta_data) {
    using namespace H5;
    DataSpace att_space(H5S_SCALAR);
    FloatType data_type(PredType::IEEE_F64LE);
    auto attribute = file.createAttribute("time_stamp", data_type, att_space);
    boost::posix_time::ptime const time_epoch(boost::gregorian::date(1970, 1, 1));

    double time = static_cast<double >((meta_data.event_time - time_epoch).total_milliseconds()) / 1000;

    attribute.write(PredType::NATIVE_DOUBLE, &time);

}


#endif //SMART_SCREEN_HDF5EVENTSTORAGE_H
