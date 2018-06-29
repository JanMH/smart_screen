
#ifndef SMART_SCREEN_SMARTAPP_H
#define SMART_SCREEN_SMARTAPP_H

#include <boost/shared_ptr.hpp>
#include <AsyncDataQueue.h>
#include <boost/signals2.hpp>
#include <DataStreamDistributor.h>
#include <EventDetector.h>
#include <DataClassifier.h>
#include "MetricProvider.h"

template<typename DataPointType>
class SmartApp {
public:
    enum Definitions {
        ON_EVENT_DETECTED_CONNECTION_POSITION = 500
    };

    SmartApp(std::shared_ptr<DynamicStreamMetaData> meta_data = std::shared_ptr<DynamicStreamMetaData>(
            new DynamicStreamMetaData),
             std::shared_ptr<AsyncDataQueue<DataPointType>> async_data_queue = std::shared_ptr<AsyncDataQueue<DataPointType>>(
                     new AsyncDataQueue<DataPointType>),
             std::shared_ptr<EventDetector<DataPointType>> event_detector = std::shared_ptr<EventDetector<DataPointType>>(
                     new EventDetector<DataPointType>));

    std::shared_ptr<AsyncDataQueue<DataPointType>> getDataQueue();

    const std::shared_ptr<DynamicStreamMetaData> &getStreamMetaData() const;

    void setStreamMetaData(const std::shared_ptr<DynamicStreamMetaData> &stream_meta_data);

    const std::shared_ptr<AsyncDataQueue<DataPointType>> &getDataQueue() const;

    void setDataQueue(const std::shared_ptr<AsyncDataQueue<DataPointType>> &data_queue);

    const std::shared_ptr<DataStreamDistributor<DataPointType>> &getDataStreamDistributor() const;

    const std::shared_ptr<EventDetector<DataPointType>> &getEventDetector() const;

    // void setEventDetector(const std::shared_ptr<EventDetector<DataPointType>> &event_detector); // hard to implement

    const std::shared_ptr<MetricProvider<DataPointType>> &getMetricProvider() const;

    // void setMetricProvider(const std::shared_ptr<MetricProvider<DataPointType>> &steady_state_worker); // hard to implement

    const std::shared_ptr<DataClassifier<DataPointType>> &getDataClassifier() const;

    void setDataClassifier(const std::shared_ptr<DataClassifier<DataPointType>> &data_classifier);

    void startThreads();
    void stopThreads();



private:
    void onEventDetected(std::shared_ptr<Event<DataPointType>> event) {
        data_classifier->pushEvent(*event);
    }

private:

    std::shared_ptr<DynamicStreamMetaData> stream_meta_data;
    std::shared_ptr<AsyncDataQueue<DataPointType>> data_queue;
    std::shared_ptr<DataStreamDistributor<DataPointType>> data_stream_distributor;
    std::shared_ptr<EventDetector<DataPointType>> event_detector;
    std::shared_ptr<MetricProvider<DataPointType>> metric_provider;
    std::shared_ptr<DataClassifier<DataPointType>> data_classifier;

};

template<typename DataPointType>
SmartApp<DataPointType>::SmartApp(std::shared_ptr<DynamicStreamMetaData> meta_data,
                                  std::shared_ptr<AsyncDataQueue<DataPointType>> async_data_queue,
                                  std::shared_ptr<EventDetector<DataPointType>> _event_detector)
        :stream_meta_data(meta_data),
         data_queue(async_data_queue),
         data_stream_distributor(new DataStreamDistributor<DataPointType>(stream_meta_data)),
         event_detector(_event_detector),
         metric_provider(new MetricProvider<DataPointType>),
         data_classifier(new DataClassifier<DataPointType>) {
    data_stream_distributor->addWorker(_event_detector);
    data_stream_distributor->addWorker(metric_provider);
    event_detector->event_detected.connect(ON_EVENT_DETECTED_CONNECTION_POSITION,
                                           boost::bind(&SmartApp<DataPointType>::onEventDetected, this, _1));

}

template<typename DataPointType>
std::shared_ptr<AsyncDataQueue<DataPointType>> SmartApp<DataPointType>::getDataQueue() {
    return data_queue;
}

template<typename DataPointType>
const std::shared_ptr<DynamicStreamMetaData> &SmartApp<DataPointType>::getStreamMetaData() const {
    return stream_meta_data;
}

template<typename DataPointType>
void SmartApp<DataPointType>::setStreamMetaData(const std::shared_ptr<DynamicStreamMetaData> &stream_meta_data) {
    this->stream_meta_data = stream_meta_data;
    this->data_stream_distributor->setStreamMetaData(stream_meta_data);
}

template<typename DataPointType>
const std::shared_ptr<AsyncDataQueue<DataPointType>> &SmartApp<DataPointType>::getDataQueue() const {
    return data_queue;
}

template<typename DataPointType>
void SmartApp<DataPointType>::setDataQueue(const std::shared_ptr<AsyncDataQueue<DataPointType>> &data_queue) {
    SmartApp::data_queue = data_queue;
}

template<typename DataPointType>
const std::shared_ptr<DataStreamDistributor<DataPointType>> &SmartApp<DataPointType>::getDataStreamDistributor() const {
    return data_stream_distributor;
}

template<typename DataPointType>
const std::shared_ptr<EventDetector<DataPointType>> &SmartApp<DataPointType>::getEventDetector() const {
    return event_detector;
}


template<typename DataPointType>
const std::shared_ptr<MetricProvider<DataPointType>> &SmartApp<DataPointType>::getMetricProvider() const {
    return metric_provider;
}

template<typename DataPointType>
const std::shared_ptr<DataClassifier<DataPointType>> &SmartApp<DataPointType>::getDataClassifier() const {
    return data_classifier;
}

template<typename DataPointType>
void SmartApp<DataPointType>::setDataClassifier(const std::shared_ptr<DataClassifier<DataPointType>> &data_classifier) {
    data_classifier = data_classifier;
}

template<typename DataPointType>
void SmartApp<DataPointType>::startThreads() {
    this->data_classifier->startClassification(ClassificationConfig());

}
template<typename DataPointType>
void SmartApp<DataPointType>::stopThreads() {
    this->data_classifier->stopAnalyzingWhenDone();

}

#endif //SMART_SCREEN_SMARTAPP_H
