
#ifndef SMART_SCREEN_DATASTREAMDISTRIBUTER_H
#define SMART_SCREEN_DATASTREAMDISTRIBUTER_H


#include <boost/shared_ptr.hpp>
#include <vector>
#include <deque>
#include "AsyncDataQueue.h"
#include "DataStreamWorker.h"

template<typename DataPointType>
class DataStreamWorker;



/**
 * @brief The DataStreamDistributor manages the data flow out of a class.
 *
 * It takes a list of DataStreamWorkers and gives each of them the DataPoints it requests. For a relatively simple example worker please refer to the MetricProvider within the smart_screen module.
 * @tparam DataPointType The type of DataPoint that we will be working with.
 */
template<typename DataPointType>
class DataStreamDistributor {
    typedef DataStreamWorker<DataPointType> DSWorker;
    typedef std::shared_ptr<DataStreamWorker<DataPointType>> DSWorker_ptr;
    typedef typename std::vector<DSWorker_ptr>::size_type vec_index_type;
    typedef typename std::deque<DataPointType>::size_type data_queue_index_type;

public: // methods
    DataStreamDistributor(std::shared_ptr<DynamicStreamMetaData> _meta_data = std::shared_ptr<DynamicStreamMetaData>(
            new DynamicStreamMetaData));

    /**
     * Adds a DataStreamWorker which can now request data. When adding a worker, it's dynamic stream meta data is set and it is called with zero data points to obtain a read info.
     * @param worker The worker that will be added
     */
    void addWorker(std::shared_ptr<DataStreamWorker<DataPointType>> worker);

    /**
     * Reads enough data so that each worker can be executed at least once.
     * @param queue The data queue from which data will be read
     * @return True on success, false if no more data is available within the queue.
     */
    bool loopWorkersOnce(std::shared_ptr<AsyncDataQueue<DataPointType>> queue);

    /**
     *
     * @returns the number of data points required so that each worker can be executed at least once
     */
    std::size_t dataPointsNeededForNextRun();

private: // methods
    void readBufferForNextRun(std::shared_ptr<AsyncDataQueue<DataPointType>> queue);

    void freeUnusedBuffer();

    bool executeWorkerWhileDataInBuffer(vec_index_type index);

    data_queue_index_type getIndexInLocalBufferforRead(vec_index_type read_index);

    void executeWorkerOnce(unsigned long index, unsigned long &local_buffer_index);

private:
    std::shared_ptr<DynamicStreamMetaData> meta_data;
public:
    const std::shared_ptr<DynamicStreamMetaData> &getStreamMetaData() const;

    void setStreamMetaData(const std::shared_ptr<DynamicStreamMetaData> &streamMetaData);

private:
    std::vector<DSWorker_ptr> workers;
    std::vector<typename DSWorker::NextReadInfos> read_infos;
    DynamicStreamMetaData::DataPointIdType current_global_buffer_pos = 0;
    std::deque<DataPointType> data_queue;


};

template<typename DataPointType>
bool DataStreamDistributor<DataPointType>::loopWorkersOnce(std::shared_ptr<AsyncDataQueue<DataPointType>> queue) {
    this->readBufferForNextRun(queue);
    bool one_worker_was_executed = false;

    for (vec_index_type j = 0; j < this->workers.size(); ++j) {
        while (executeWorkerWhileDataInBuffer(j)) {
            one_worker_was_executed = true;
        }
    }
    freeUnusedBuffer();
    return one_worker_was_executed;
}

template<typename DataPointType>
void DataStreamDistributor<DataPointType>::addWorker(std::shared_ptr<DataStreamWorker<DataPointType>> worker) {
    worker->setStreamMetaData(this->meta_data);
    workers.push_back(worker);
    read_infos.push_back(worker->workOnDataPoints(data_queue.begin(), data_queue.begin(), current_global_buffer_pos));
}

template<typename DataPointType>
bool DataStreamDistributor<DataPointType>::executeWorkerWhileDataInBuffer(
        DataStreamDistributor<DataPointType>::vec_index_type index) {
    bool read_stuff = false;
    auto local_buffer_index = getIndexInLocalBufferforRead(index);
    while (data_queue.size() - local_buffer_index >= read_infos[index].read_number_of_data_points) {
        executeWorkerOnce(index, local_buffer_index);
        read_stuff = true;

    }
    return read_stuff;
}

template<typename DataPointType>
DataStreamDistributor<DataPointType>::DataStreamDistributor(std::shared_ptr<DynamicStreamMetaData> _meta_data)
        :meta_data(_meta_data) {

}

template<typename DataPointType>
void DataStreamDistributor<DataPointType>::executeWorkerOnce(unsigned long index, unsigned long &local_buffer_index) {
    auto begin = data_queue.begin() + local_buffer_index;
    auto actual_end = std::min(read_infos[index].read_number_of_data_points, data_queue.size() - local_buffer_index);

    read_infos[index] = workers[index]->workOnDataPoints(begin,
                                                         begin + actual_end,
                                                         current_global_buffer_pos + local_buffer_index);
    local_buffer_index = getIndexInLocalBufferforRead(index);
}

template<typename DataPointType>
typename DataStreamDistributor<DataPointType>::data_queue_index_type
DataStreamDistributor<DataPointType>::getIndexInLocalBufferforRead(
        DataStreamDistributor<DataPointType>::vec_index_type read_index) {
    if (read_infos[read_index].next_read_location < current_global_buffer_pos) {
        return 0;
    }
    auto result = static_cast<DataStreamDistributor<DataPointType>::data_queue_index_type>(
            DynamicStreamMetaData::DataPointIdType(
                    read_infos[read_index].next_read_location - current_global_buffer_pos)
    );
    return result;
}

template<typename DataPointType>
void DataStreamDistributor<DataPointType>::readBufferForNextRun(std::shared_ptr<AsyncDataQueue<DataPointType>> queue) {
    auto data_points_still_needed = dataPointsNeededForNextRun();

    if (data_points_still_needed <= 0) { // need not read any points
        return;
    }
    auto data_queue_pos = this->data_queue.size();
    auto new_queue_size = this->data_queue.size() + data_points_still_needed;
    this->data_queue.resize(new_queue_size);

    auto new_end = queue->popDataPoints(this->data_queue.begin() + data_queue_pos, this->data_queue.end());
    this->data_queue.erase(new_end,this->data_queue.end());

}

template<typename DataPointType>
void DataStreamDistributor<DataPointType>::freeUnusedBuffer() {
    auto min_needed = std::min_element(read_infos.begin(), read_infos.end(), [](const auto &cmp1, const auto &cmp2) {
        return cmp1.next_read_location - cmp1.keep_number_of_data_points_available <
               cmp2.next_read_location - cmp2.keep_number_of_data_points_available;
    });
    auto earliest_pos_needed = min_needed->next_read_location - min_needed->keep_number_of_data_points_available;
    if (earliest_pos_needed <= current_global_buffer_pos) {
        return;
    }
    DynamicStreamMetaData::DataPointIdType elements_erased = earliest_pos_needed - current_global_buffer_pos;
    this->data_queue.erase(this->data_queue.begin(),
                           this->data_queue.begin() + static_cast<data_queue_index_type>( elements_erased));
    this->current_global_buffer_pos = earliest_pos_needed;
}

template<typename DataPointType>
std::size_t DataStreamDistributor<DataPointType>::dataPointsNeededForNextRun() {
    auto max_read = std::max_element(read_infos.begin(), read_infos.end(), [](const auto &cmp1, const auto &cmp2) {
        return cmp1.next_read_location + cmp1.read_number_of_data_points <
               cmp2.next_read_location + cmp2.read_number_of_data_points;
    });
    DynamicStreamMetaData::DataPointIdType data_points_still_needed =
            max_read->next_read_location + max_read->read_number_of_data_points - this->data_queue.size() -
            this->current_global_buffer_pos;
    return static_cast<std::size_t>(data_points_still_needed);
}

template<typename DataPointType>
const std::shared_ptr<DynamicStreamMetaData> &DataStreamDistributor<DataPointType>::getStreamMetaData() const {

    return meta_data;
}

template<typename DataPointType>
void
DataStreamDistributor<DataPointType>::setStreamMetaData(const std::shared_ptr<DynamicStreamMetaData> &streamMetaData) {
    meta_data = streamMetaData;
    for (auto &worker: workers) {
        worker->setStreamMetaData(streamMetaData);
    }
}


#endif //SMART_SCREEN_DATASTREAMDISTRIBUTER_H
