
#ifndef SMART_SCREEN_DATA_STREAM_GENERATORS_H
#define SMART_SCREEN_DATA_STREAM_GENERATORS_H

#include <cstddef>
#include <memory>
#include <thread>
#include <boost/serialization/vector.hpp>
#include "AsyncDataQueue.h"

template<typename DataPointType>
class DataStreamGenerator {
public:
    virtual void
    startReadingInExtraThread(std::shared_ptr<AsyncDataQueue<DataPointType>> data_queue,
                              std::size_t number_of_data_points,
                              std::size_t chunk_size = 200);

    virtual void
    generateNextDataPoints(std::shared_ptr<AsyncDataQueue<DataPointType>> data_queue,
                           std::size_t number_of_data_points) =0;

private:
    void run(std::shared_ptr<AsyncDataQueue<DataPointType>> data_queue, std::size_t number_of_data_points,
             std::size_t chunk_size);

    std::thread runner;
};

template<typename DataPointType>
void
DataStreamGenerator<DataPointType>::startReadingInExtraThread(std::shared_ptr<AsyncDataQueue<DataPointType>> data_queue,
                                                              std::size_t number_of_data_points,
                                                              std::size_t chunk_size) {
    runner = std::thread(&DataStreamGenerator<DataPointType>::run, this, data_queue, number_of_data_points, chunk_size);

}

template<typename DataPointType>
void
DataStreamGenerator<DataPointType>::run(std::shared_ptr<AsyncDataQueue<DataPointType>> data_queue,
                                        std::size_t number_of_data_points,
                                        std::size_t chunk_size) {
    while (number_of_data_points > chunk_size) {
        number_of_data_points -= chunk_size;
        this->generateNextDataPoints(data_queue, chunk_size);
    }
    if (number_of_data_points != 0) {
        this->generateNextDataPoints(data_queue, number_of_data_points);
    }
    data_queue->notifyStreamEnd();
}

class CountingGenerator : public DataStreamGenerator<DefaultDataPoint> {
public:
    virtual void generateNextDataPoints(std::shared_ptr<AsyncDataQueue<DefaultDataPoint>> data_queue,
                                        std::size_t number_of_data_points) {
        std::vector<DefaultDataPoint> inserted;
        while (number_of_data_points > 0) {
            inserted.push_back(DefaultDataPoint(start_val, start_val));
            start_val += 1;
            --number_of_data_points;
        }
        data_queue->addDataPoints(inserted.begin(), inserted.end());
    }

private:
    float start_val = 0;
};

class StationaryGenerator : public DataStreamGenerator<DefaultDataPoint> {
public:
    StationaryGenerator(DefaultDataPoint data_point = DefaultDataPoint(0, 0)) : dp(data_point) {}

    virtual void generateNextDataPoints(std::shared_ptr<AsyncDataQueue<DefaultDataPoint>> data_queue,
                                        std::size_t number_of_data_points) {
        std::vector<DefaultDataPoint> inserted;
        inserted.insert(inserted.begin(), number_of_data_points, dp);
        data_queue->addDataPoints(inserted.begin(), inserted.end());
    }

private:
    DefaultDataPoint dp;
};

class SwitchingGenerator : public DataStreamGenerator<DefaultDataPoint> {
    std::size_t switch_after;
    DefaultDataPoint a, b;
    std::size_t total_number_of_data_points = 0;
public:
    SwitchingGenerator(std::size_t _switch_after = 50,DefaultDataPoint _a = DefaultDataPoint(0, 0), DefaultDataPoint _b = DefaultDataPoint(20, 20))
            :switch_after(_switch_after), a(_a), b(_b) {}
    virtual void generateNextDataPoints(std::shared_ptr<AsyncDataQueue<DefaultDataPoint>> data_queue,
                                        std::size_t number_of_data_points) {
        while (number_of_data_points > 0) {
            auto number_of_data_points_inserted =std::min(switch_after- total_number_of_data_points % switch_after,number_of_data_points);
            auto cond = total_number_of_data_points % (2*switch_after);
            if( cond< switch_after) {
                insertDataPoints(data_queue,number_of_data_points_inserted,a);
            } else {

                insertDataPoints(data_queue,number_of_data_points_inserted,b);
            }
            number_of_data_points -= number_of_data_points_inserted;
            total_number_of_data_points += number_of_data_points_inserted;
        }
    }

private:
    void insertDataPoints(std::shared_ptr<AsyncDataQueue<DefaultDataPoint>> data_queue, std::size_t number_of_data_points, DefaultDataPoint inserted) {
        std::vector<DefaultDataPoint> tmp_vec;
        tmp_vec.insert(tmp_vec.begin(), number_of_data_points, inserted);
        data_queue->addDataPoints(tmp_vec.begin(), tmp_vec.end());


    }
};

#endif //SMART_SCREEN_DATA_STREAM_GENERATORS_H
