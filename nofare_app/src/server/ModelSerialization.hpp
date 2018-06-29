
#ifndef SMART_SCREEN_MODELSERIALIZATION_HPP
#define SMART_SCREEN_MODELSERIALIZATION_HPP

#include <DataClassifier.h>
#include <atomic>
#include <boost/archive/text_oarchive.hpp>
#include <SerializeEventLabelManager.h>

#define OLD_SERIALIZED_PATH  "model.archive.backup"
#define NEW_SERIALIZED_PATH  "model.archive"


namespace detail {
    extern std::atomic_bool shutdown;
    extern std::thread model_thread;
    extern std::mutex condition_mutex;
    extern std::condition_variable wakeup_var;
}

template<typename DataPointType>
void storeModelThreadFunction(std::shared_ptr<DataClassifier<DataPointType>> stored, const std::string old_path,
                              const std::string new_path) {


    while (!detail::shutdown ) {
        std::unique_lock<std::mutex> lk(detail::condition_mutex);
        detail::wakeup_var.wait_for(lk, std::chrono::seconds(10));

        program_log->info("storing model to {}", old_path);
        std::ofstream result_serialization_stream(old_path, std::ofstream::trunc);
        boost::archive::text_oarchive ia(result_serialization_stream);
        auto labelManager = stored->getEventLabelManager();
        ia << labelManager;
        std::remove(new_path.c_str());
        program_log->info("moving model to {}", new_path);
        std::rename(old_path.c_str(), new_path.c_str());
    }

}

template<typename DataPointType>
void startModelStorageThread(std::shared_ptr<DataClassifier<DataPointType>> stored) {

    detail::shutdown = false;
    auto old_path = OLD_SERIALIZED_PATH;
    auto new_path = NEW_SERIALIZED_PATH;
    detail::model_thread = std::thread(storeModelThreadFunction<DataPointType>, stored, old_path, new_path);

}

template<typename DataPointType>
void stopModelStorageThread() {
    detail::shutdown = true;
    detail::wakeup_var.notify_all();

    detail::model_thread.join();

}

#endif //SMART_SCREEN_MODELSERIALIZATION_HPP
