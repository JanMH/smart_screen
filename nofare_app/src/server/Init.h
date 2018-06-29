
#ifndef SMART_SCREEN_INIT_HPP
#define SMART_SCREEN_INIT_HPP

#include <iostream>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <algorithm>
#include <string>
#include <ProgramConfiguration.h>
#include <BluedHdf5InputSource.h>
#include <AddLabelCommand.h>
#include <ShutdownCommand.h>

#include "Logger.h"

#include "DefaultDataPoint.h"
#include "SmartApp.h"

#include "EventHandlers.h"

#include "CLIConnection.h"
#include "nofare_definitions.h"
#include "InputSource.h"
#include "ModelSerialization.hpp"

#include <fstream>
#include <boost/archive/text_iarchive.hpp>
#include <ReclassifyCommand.h>

#include "NoFaReAppConfig.h"




#ifdef USE_BLUED
typedef BluedDataPoint ConfiguredDataPoint;
#else
typedef DefaultDataPoint ConfiguredDataPoint;
#endif




extern SmartApp<ConfiguredDataPoint> smart_app;

extern std::atomic_bool shutdown_nofare_app;

template <typename DataPointType>
void messageLoop();

inline void loadOldModel(SmartApp<ConfiguredDataPoint>& to_add_to) {
    try {

        // create and open an archive for input
        std::ifstream ifs(NEW_SERIALIZED_PATH);
        boost::archive::text_iarchive ia(ifs);

        EventLabelManager<ConfiguredDataPoint> mgr;
        // read class state from archive
        ia >> mgr;
        to_add_to.getDataClassifier()->setEventLabelManager(mgr);
    } catch (...) {
        program_log->warn("Could not load the old model. Is this the first start.");
    }
}
template <typename DataPointType>
bool run(const char *config_file_path, const char *blued_file = nullptr) {

    ProgramConfiguration config;
    if (!config.load(config_file_path)) {
        std::cerr << "Could not load config\n";
        return -1;
    }

    if (!setupMessageQueue()) {
        return false;
    }




    auto data_queue = smart_app.getDataQueue();
    auto dynamic_meta_data = smart_app.getStreamMetaData();
    dynamic_meta_data->setFixedPowerMetaData(config.toPowerMetaData());
    smart_app.getEventDetector()->setThreshold(0.02);

    loadOldModel(smart_app);

#ifdef USE_BLUED
    program_log->info("Using blued");
    BluedHdf5InputSource input_source;
    input_source.startReading(data_queue, dynamic_meta_data, blued_file, config.data_set_start_time);
#else
    program_log->info("Using custom input source");
    InputSource input_source;
    input_source.startReading(data_queue, dynamic_meta_data);
#endif


    std::thread messageQueueThread(&messageLoop<DataPointType>);

    connectEvents<DataPointType>(smart_app);
    smart_app.startThreads();
    startModelStorageThread(smart_app.getDataClassifier());
    while (!shutdown_nofare_app) {
        try {
            if (!smart_app.getDataStreamDistributor()->loopWorkersOnce(data_queue)) {
                break;
            }
        } catch (...) {
            program_log->error("an exception was thrown...");
        }
    }
    input_source.stopNow();

    smart_app.stopThreads();
    stopModelStorageThread<DataPointType>();
    if (messageQueueThread.joinable() &&
        shutdown_nofare_app) { // if we had a clean shutdown, join the thread. Otherwise just kill it
        messageQueueThread.join();
    }
    program_log->info("data stream ended...");
    cleanUp(0);
    return 0;
}

template <typename DataPointType>
void messageLoop() {
    while (true) {
        std::size_t received_size;
        unsigned int priority;
        char message[MAX_MESSAGE_SIZE];
        cli_message_queue->receive(message, sizeof(message), received_size, priority);

        std::string commandStr(message, message + received_size);
        if (AddLabelCommand::isAddLabelCommand(commandStr)) {
            AddLabelCommand commandData = AddLabelCommand::deserialize(commandStr);
            program_log->info(" adding event label {} for id {}", commandData.label, commandData.event_id);
            if (smart_app.getDataClassifier()->addLabelById(
                    static_cast<EventMetaData::EventIdType>(commandData.event_id), commandData.label)) {
                program_log->info("Successfully added label to event");
            } else {
                program_log->error("adding label {} to event {} failed. No such event.", commandData.label,
                                   commandData.event_id);
            }
        } else if (ReclassifyCommand::isReclassifyCommand(commandStr)) {
            program_log->info("received a reclassify command");
            ReclassifyCommand commandData = ReclassifyCommand::deserialize(commandStr);
            smart_app.getDataClassifier()->reclassify(commandData.event_id);
        }
        else if (ShutdownCommand::isShutDownCommand(commandStr)) {
            program_log->info("received a shutdown command");
            shutdown_nofare_app = true;
            return;
        }

    }
}



#endif //SMART_SCREEN_INIT_HPP
