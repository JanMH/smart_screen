#ifndef _DATAMANAGER_H_
#define _DATAMANAGER_H_

#include <deque>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <boost/optional.hpp>
#include <algorithm>


#include "DefaultDataPoint.h"
#include "PowerMetaData.h"

/**
 * @brief The DataManager is a synchronized point for asynchronous reading and writing operations.
 *
 */
template<typename DataPointType> class AsyncDataQueue;

typedef AsyncDataQueue<DefaultDataPoint> DefaultAsyncDataQueue;


/**
 * @brief The async data queue provides a fast safe way to exchange large chunks of data between two threads.
 * @tparam DataPointType The type of data point that will be exchanged
 */
template<typename DataPointType> class AsyncDataQueue {
public:
    ~AsyncDataQueue();

    /**
     * @brief This function sets the maximum amount of data that is allowed to be in the queue until it blocks insert operations
     * @param max_size The maximum number of elements
     */
    void setQueueMaxSize(unsigned long max_size);

    /**
     *
     * @returns The maximum amount of data points this queue may contain
     */
    unsigned long getQueueMaxSize();

    /**
     *
     * @return Gets the current amount of data points within the queue
     */

    unsigned long getQueueSize();




    /**
     * @brief Fills the iterator with data points. If the stream writing to this DataManager notifies the datamanager that no more data will be following, the returning iterator will point to the position after the last element.
     *
     * @param begin Iterator to first writing destination.
     * @param end Iterator end. If the running iterator equals end, no more data will be written.
     *
     */
    template<class IteratorType> IteratorType
    getDataPoints(IteratorType begin, IteratorType end, unsigned long offset = 0);


    /**
     * @brief Removes num_data_points DataPoints from the queue
     *
     * @param num_data_points Number of data points to be removed.
     *
     */
    void nextDataPoints(unsigned long num_data_points);

    /**
     * @brief Transfers datapoints from the queue to the specified range.
     * @param begin Iterator to first writing destination.
     * @param end Iterator end. If the running iterator equals end, no more data will be written.
     *
     */
    template<class IteratorType> IteratorType popDataPoints(IteratorType begin, IteratorType end);


    /**
     * @brief Adds a datapoint to the back of the queue. Blocks until less than PowerMeterData::max_data_points_in_queue is in the queue.
     *
     * @param data_point The datapoint that is to be added.
     *
     */
    void addDataPoint(DataPointType data_point);

    /**
     * @brief Adds multiple data points to the queue.
     *
     * @param begin Starting iterator
     * @param end Iterator past the end of the buffer
     *
     */
    template<typename IteratorType> void addDataPoints(IteratorType begin, IteratorType end);


    /**
     * @brief Unblocks calls to getDataPoints and nextData by safely returning the data already read for the former and removing the maximum possible amount of data for the latter.
     *
     */
    void notifyStreamEnd();

    /**
     * @brief  This function can be used after notifyStreamEnd to restart the normal operation of the queu
     */
    void restartStreaming() {
        this->stream_ended = false;
    }


    /**
     * @brief Does the same as notifyStreamEnd
     */
    void unblockReadOperations() {
        this->notifyStreamEnd();
    }

    /**
     * @brief  This function discards the rest of the queue to unlock the writing operations. This operation can be used for a fast shutdown of the program.
     */
    void discardRestOfStream() {
        std::unique_lock<std::mutex> lock(data_queue_mutex);
        this->data_queue.clear();
        this->stream_ended = true;
        this->deque_overflow.notify_all();
    }


private:
    std::function<bool()> getQueueHasEnoughElementsWaiter(unsigned long num_elements);

    std::function<bool()> getQueueNotFullWaiter();

    std::function<bool()> getQueueNotEmptyWaiter();

    void removePointsFromQueue(unsigned long num_data_points);


private:
    std::condition_variable deque_overflow;
    std::condition_variable deque_underflow;
    std::mutex data_queue_mutex;
    std::deque<DataPointType> data_queue;
    bool stream_ended = false;

    unsigned long queue_max_size = 4096;
};


template<typename DataPointType> AsyncDataQueue<DataPointType>::~AsyncDataQueue() {
    {
        std::lock_guard<std::mutex> clear_queue(data_queue_mutex);
        data_queue.clear();
    }
    deque_overflow.notify_all();
}

template<typename DataPointType> unsigned long AsyncDataQueue<DataPointType>::getQueueMaxSize() {
    return this->queue_max_size;
}

template<typename DataPointType> unsigned long AsyncDataQueue<DataPointType>::getQueueSize() {
    std::unique_lock<std::mutex> queue_lock(this->data_queue_mutex);

    return this->data_queue.size();
}

template<typename DataPointType> void AsyncDataQueue<DataPointType>::setQueueMaxSize(unsigned long max_size) {
    std::unique_lock<std::mutex> queue_lock(this->data_queue_mutex);
    this->queue_max_size = max_size;
    this->deque_overflow.notify_one();
}


template<typename DataPointType> template<typename IteratorType> void
AsyncDataQueue<DataPointType>::addDataPoints(IteratorType begin, IteratorType end) {
    auto waiting_function = this->getQueueNotFullWaiter();

    while (begin != end && !this->stream_ended) {
        std::unique_lock<std::mutex> deque_overflow_wait_lock(this->data_queue_mutex);
        this->deque_overflow.wait(deque_overflow_wait_lock, waiting_function);
        unsigned long pushable_elements = this->queue_max_size - this->data_queue.size();
        unsigned long elements_pushed = std::min(static_cast<unsigned long>(end - begin), pushable_elements);
        this->data_queue.insert(data_queue.end(), begin, begin + elements_pushed);
        begin += elements_pushed;
        this->deque_underflow.notify_one();
    }
}


template<typename DataPointType> template<typename IteratorType> IteratorType
AsyncDataQueue<DataPointType>::getDataPoints(IteratorType begin, IteratorType end, unsigned long offset) {
    auto waiting_function = this->getQueueHasEnoughElementsWaiter(static_cast<long>(end - begin) + offset);
    std::unique_lock<std::mutex> deque_overflow_wait_lock(this->data_queue_mutex);
    this->deque_underflow.wait(deque_overflow_wait_lock, waiting_function);
    long pullable_elements = this->data_queue.size() - offset;
    long elements_pulled = std::min(static_cast<long>(end - begin), pullable_elements);
    begin = std::copy_n(data_queue.begin() + offset, elements_pulled, begin);
    return begin;
}

template<typename DataPointType> void AsyncDataQueue<DataPointType>::nextDataPoints(unsigned long num_data_points) {
    std::unique_lock<std::mutex> deque_overflow_wait_lock(this->data_queue_mutex);

    auto queue_has_enough_elements = this->getQueueHasEnoughElementsWaiter(num_data_points);
    this->deque_underflow.wait(deque_overflow_wait_lock, queue_has_enough_elements);
    this->removePointsFromQueue(num_data_points);
    this->deque_overflow.notify_one();
}


template<typename DataPointType> void AsyncDataQueue<DataPointType>::addDataPoint(DataPointType data_point) {
    std::unique_lock<std::mutex> deque_overflow_wait_lock(this->data_queue_mutex);

    auto queue_not_full = this->getQueueNotFullWaiter();
    this->deque_overflow.wait(deque_overflow_wait_lock, queue_not_full);

    this->data_queue.push_back(data_point);
    this->deque_underflow.notify_one();

}

template<typename DataPointType> std::function<bool()>
AsyncDataQueue<DataPointType>::getQueueHasEnoughElementsWaiter(unsigned long num_elements) {
    return [this, num_elements]() -> bool {
        return this->data_queue.size() >= num_elements || this->stream_ended;
    };
}

template<typename DataPointType> std::function<bool()> AsyncDataQueue<DataPointType>::getQueueNotEmptyWaiter() {
    return [this]() -> bool {
        auto size = this->data_queue.size();
        auto result = size > 0 || this->stream_ended;
        return result;
    };
}

template<typename DataPointType> std::function<bool()> AsyncDataQueue<DataPointType>::getQueueNotFullWaiter() {
    return [this]() -> bool {
        return this->data_queue.size() < this->queue_max_size || this->stream_ended;
    };
}

template<typename DataPointType> void AsyncDataQueue<DataPointType>::notifyStreamEnd() {
    this->stream_ended = true;
    this->deque_underflow.notify_one();
}

template<typename DataPointType> template<class IteratorType> IteratorType
AsyncDataQueue<DataPointType>::popDataPoints(IteratorType begin, IteratorType end) {
    auto waiting_function = this->getQueueNotEmptyWaiter();
    do {
        std::unique_lock<std::mutex> deque_underflow_wait_lock(this->data_queue_mutex);

        this->deque_underflow.wait(deque_underflow_wait_lock, waiting_function);

        long pullable_elements = this->data_queue.size();
        long elements_pulled = std::min(static_cast<long>(end - begin), pullable_elements);
        begin = std::copy_n(data_queue.begin(), elements_pulled, begin);
        this->removePointsFromQueue(elements_pulled);
        this->deque_overflow.notify_one();

    } while (begin != end && !this->stream_ended);
    return begin;
}

template<typename DataPointType> void AsyncDataQueue<DataPointType>::removePointsFromQueue(unsigned long num_data_points) {
    if (num_data_points >= this->data_queue.size()) {
        data_queue.clear();
        return;
    }
    this->data_queue.erase(data_queue.begin(), data_queue.begin() + num_data_points);


}


#endif // _DATAMANAGER_H_
