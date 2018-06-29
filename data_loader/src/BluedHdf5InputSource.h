#ifndef SMART_SCREEN_BLUEDHDF5INPUTSOURCE_H
#define SMART_SCREEN_BLUEDHDF5INPUTSOURCE_H

#include <functional>
#include <thread>
#include "DynamicStreamMetaData.h"
#include <H5Cpp.h>

#include "BluedDefinitions.h"

/**
 * @brief This class reads BluedDataPoints from a file that was created using the blued_converter
 */
class BluedHdf5InputSource {
public:

    /**
     * Reads the blued data set in a separate thread.
     * @param data_queue Data will be stored in this data_queue
     * @param _meta_data The dynamic stream meta data with which the time points will be synchronized
     * @param file_path The path to the file from which we will read the data
     * @param data_set_start_time The data set start time. Blued data points all contain a timestamp that offsets from this time.
     */
    void startReading(std::shared_ptr<AsyncDataQueue<BluedDataPoint>> data_queue,
                          std::shared_ptr<DynamicStreamMetaData> _meta_data,
                          const std::string &file_path, const std::string& data_set_start_time);

    /**
     * @brief Immediately stops the thread
     */
    void stopNow();

    /**
     * @brief Stops the thread without discarding any data from the data queue
     */
    void stopGracefully();

    ~BluedHdf5InputSource() {
        this->stopNow();
        this->stopGracefully();
    }


public:
    BluedDataManager data_manager;
    std::shared_ptr<DynamicStreamMetaData> meta_data;


private:
    void run(const std::string &file_path);

    bool readOnce(H5::DataSet dataset, H5::DataSpace &dataspace);
    void updateDynamicStreamMetaData(BluedDataPoint to_update);
    void initStartValues();
    void writeBufferToDataSet(unsigned int num_data_points);

private:
    bool continue_reading = true;
    std::thread runner;
    hsize_t data_set_size[2];
    hsize_t current_offset[2];
    static const unsigned int buffer_size = 1000;
    static const int fields = 4;

    float buffer[buffer_size][fields];
    DynamicStreamMetaData::TimeType start_time;

    hsize_t mem_space_dimensions[2] = {buffer_size, fields};

    H5::DataSpace memspace = H5::DataSpace(2, mem_space_dimensions);

};


#endif //SMART_SCREEN_BLUEDHDF5INPUTSOURCE_H
