#include <boost/test/unit_test.hpp>
#include <DefaultDataPoint.h>
#include <Hdf5EventStorage.h>

BOOST_AUTO_TEST_CASE(serialize_event) {
    std::vector<DefaultDataPoint> data_points(500);
    int counter = 0;
    std::generate(data_points.begin(), data_points.end(), [&counter]() {
        counter++;
        return DefaultDataPoint(counter, counter);
    });
    Event<DefaultDataPoint> ev;
    ev.event_data = data_points;

    Hdf5EventStorage<DefaultDataPoint> storage;
    storage.storeEvent(ev);
}
