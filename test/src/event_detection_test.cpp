#include <boost/test/unit_test.hpp>
#include <SmartApp.h>
#include <BluedDataPoint.h>
#include <BluedHdf5InputSource.h>
#include <EventDetector.h>
#include "DataStreamDistributor.h"
#include "data_stream_generators.h"

struct DetectionSetup {
    std::shared_ptr<AsyncDataQueue<DefaultDataPoint>> queue;

    DataStreamDistributor<DefaultDataPoint> distributor;

    std::shared_ptr<EventDetector<DefaultDataPoint>> worker;

    std::shared_ptr<DynamicStreamMetaData> stream_meta_data;

    DetectionSetup(float detection_threshold = 0.2, float current_period_weight = 0.6,
                   DynamicStreamMetaData::DataPointIdType detection_chunk_size = 200) {
        queue = std::shared_ptr<AsyncDataQueue<DefaultDataPoint>>(new AsyncDataQueue<DefaultDataPoint>());
        worker = std::shared_ptr<EventDetector<DefaultDataPoint>>(
                new EventDetector<DefaultDataPoint>(detection_threshold,  detection_chunk_size));
        stream_meta_data = std::shared_ptr<DynamicStreamMetaData>(new DynamicStreamMetaData);
        worker->setStreamMetaData(stream_meta_data);

        distributor.addWorker(worker);

    }
};


BOOST_AUTO_TEST_CASE(no_detection_test) {
    StationaryGenerator data_steam_gen;
    DetectionSetup setup;
    data_steam_gen.generateNextDataPoints(setup.queue, 200);

    bool event_was_detected = false;

    setup.worker->event_detected.connect([&event_was_detected](auto bla) { event_was_detected = true; });
    setup.distributor.loopWorkersOnce(setup.queue);
    BOOST_TEST(!event_was_detected);
}

BOOST_AUTO_TEST_CASE(test_switching_generator) {
    DefaultDataPoint a(0, 0), b(20, 20);
    std::size_t chunk_size = 200;
    SwitchingGenerator data_steam_gen(chunk_size);
    std::shared_ptr<AsyncDataQueue<DefaultDataPoint>> queue(new AsyncDataQueue<DefaultDataPoint>);
    data_steam_gen.generateNextDataPoints(queue, 2000);
    for (int i = 0; i < 10; ++i) {
        std::vector<DefaultDataPoint> dps(chunk_size);
        queue->popDataPoints(dps.begin(), dps.end());
        if (i % 2) {
            BOOST_TEST(std::all_of(dps.begin(), dps.end(),
                                   [b](auto &data_point) { return data_point == b; }));
        } else {
            BOOST_TEST(std::all_of(dps.begin(), dps.end(),
                                   [a](auto &data_point) { return data_point == a; }));
        }
    }


}

BOOST_AUTO_TEST_CASE(detection_test) {
    DefaultDataPoint a(0, 0), b(20, 20);
    SwitchingGenerator data_steam_gen(2000);
    DetectionSetup setup;
    auto fdata = setup.stream_meta_data->getFixedPowerMetaData();
    fdata.data_points_stored_of_event = 600;
    fdata.data_points_stored_before_event = 400;
    setup.stream_meta_data->setFixedPowerMetaData(fdata);
    setup.queue->setQueueMaxSize(200000);

    data_steam_gen.generateNextDataPoints(setup.queue, 20000);

    bool event_was_detected = false;

    setup.worker->event_detected.connect([&event_was_detected, a, b](std::shared_ptr<Event<DefaultDataPoint>> bla) {
        Event<DefaultDataPoint>& ev = *bla;
        BOOST_TEST(std::all_of(bla->before_event_begin(), bla->before_event_end(),
                               [a](auto &data_point) { return data_point == a; }));
        event_was_detected = true;
        BOOST_TEST(std::all_of(bla->event_begin(), bla->event_end(),
                               [b](auto &data_point) { return data_point == b; }));
    });
    setup.queue->notifyStreamEnd();

    while(setup.distributor.loopWorkersOnce(setup.queue))
        ;// do nothing

    BOOST_TEST(event_was_detected);
}

