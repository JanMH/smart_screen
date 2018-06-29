#include <boost/test/unit_test.hpp>
#define private public
#include <DataStreamWorker.h>
#include <DataStreamDistributor.h>
#include "data_stream_generators.h"

template<typename DataPointType>
class SimpleCountingWorker : public DataStreamWorker<DataPointType> {
public:
    SimpleCountingWorker(std::size_t _read_chunk_size = 50) : read_chunk_size(_read_chunk_size) {}

    virtual typename DataStreamWorker<DataPointType>::NextReadInfos
    workOnDataPoints(typename DataStreamTypes<DataPointType>::DataStreamIterator begin,
                     typename DataStreamTypes<DataPointType>::DataStreamIterator end,
                     DynamicStreamMetaData::DataPointIdType data_point_id);

private:
    std::size_t read_chunk_size;
    float prev = -1.f;
    typename DataStreamWorker<DataPointType>::NextReadInfos requested_infos;
};

template<typename DataPointType>
typename DataStreamWorker<DataPointType>::NextReadInfos
SimpleCountingWorker<DataPointType>::workOnDataPoints(typename DataStreamTypes<DataPointType>::DataStreamIterator begin,
                                                      typename DataStreamTypes<DataPointType>::DataStreamIterator end,
                                                      DynamicStreamMetaData::DataPointIdType data_point_id) {
    requested_infos = typename DataStreamWorker<DataPointType>::NextReadInfos(0, read_chunk_size,
                                                                              requested_infos.next_read_location +
                                                                              (end - begin));
    while (begin != end) {
        BOOST_TEST(begin->ampere() == prev + 1);
        prev += 1;
        ++begin;
    }
    return requested_infos;
}


BOOST_AUTO_TEST_CASE(DataStreamDistributor_counting_test) {

    CountingGenerator data_steam_gen;
    std::shared_ptr<AsyncDataQueue<DefaultDataPoint>> queue(new AsyncDataQueue<DefaultDataPoint>());
    data_steam_gen.generateNextDataPoints(queue, 50);

    DataStreamDistributor<DefaultDataPoint> distributor;
    std::shared_ptr<DataStreamWorker<DefaultDataPoint>> worker(new SimpleCountingWorker<DefaultDataPoint>());
    distributor.addWorker(worker);
    distributor.loopWorkersOnce(queue);

}

BOOST_AUTO_TEST_CASE(DataStreamDistributor_test_multiple_workers) {

    std::size_t number_of_data_points_tested = 1000;
    std::size_t max_chunk_size = 50;
    CountingGenerator data_steam_gen;
    std::shared_ptr<AsyncDataQueue<DefaultDataPoint>> queue(new AsyncDataQueue<DefaultDataPoint>());
    data_steam_gen.generateNextDataPoints(queue, number_of_data_points_tested);


    DataStreamDistributor<DefaultDataPoint> distributor;
    for (int i = 0; i < 10; ++i) {

        std::shared_ptr<DataStreamWorker<DefaultDataPoint>> worker(
                new SimpleCountingWorker<DefaultDataPoint>(max_chunk_size- i*2));
        distributor.addWorker(worker);
    }
    for (uint i = 0; i < number_of_data_points_tested / max_chunk_size; ++i) {
        distributor.loopWorkersOnce(queue);
        BOOST_TEST(distributor.data_queue.size() < max_chunk_size*2);
    }


}