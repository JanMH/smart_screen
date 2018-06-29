
#include "InputSource.h"


void InputSource::startReading(std::shared_ptr<AsyncDataQueue<DefaultDataPoint>> _data_queue,
                               std::shared_ptr<DynamicStreamMetaData> _stream_meta_data) {
    this->data_queue = _data_queue;
    this->stream_meta_data = _stream_meta_data;
    this->runner = std::thread(&InputSource::readDataPoints, this);
}

void InputSource::readDataPoints() {
    std::vector<DefaultDataPoint> buffer;
    const int buffer_size = 1000;
    buffer.resize(buffer_size);

    bool can_read_data = true;
    while (can_read_data) {

        for (int i = 0; i < buffer_size; ++i) {

            // read valid data here... please modify
            DefaultDataPoint::datum volts = 0, ampere = 0;
            buffer[i] = DefaultDataPoint(volts, ampere);
            ++data_points_read;
            // possibly add a condition that changes can_read_data to false
        }
        auto last_data_point_read_time = boost::posix_time::microsec_clock::universal_time();

        this->data_queue->addDataPoints(buffer.begin(), buffer.end()); // adds a chunk of data to the data queue
        this->stream_meta_data->syncTimePoint(data_points_read, last_data_point_read_time);
    }
}

