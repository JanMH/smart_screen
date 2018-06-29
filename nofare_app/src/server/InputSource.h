
#ifndef SMART_SCREEN_INPUTSOURCE_H
#define SMART_SCREEN_INPUTSOURCE_H


#include <memory>
#include <AsyncDataQueue.h>
#include <thread>
#include <DynamicStreamMetaData.h>

class InputSource {
    std::thread runner;
    std::shared_ptr<AsyncDataQueue<DefaultDataPoint>> data_queue;
    std::shared_ptr<DynamicStreamMetaData> stream_meta_data;
    DynamicStreamMetaData::DataPointIdType data_points_read;

public:
void startReading(std::shared_ptr<AsyncDataQueue<DefaultDataPoint>> data_queue,
                  std::shared_ptr<DynamicStreamMetaData> stream_meta_data);
    void stopNow() {
        runner = std::thread();
    }
private:
    void readDataPoints();

};


#endif //SMART_SCREEN_INPUTSOURCE_H
