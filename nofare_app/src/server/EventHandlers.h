
#ifndef SMART_SCREEN_EVENTHANDLERS_H
#define SMART_SCREEN_EVENTHANDLERS_H

#include <SmartApp.h>
#include <boost/archive/text_oarchive.hpp>
#include "Hdf5EventStorage.h"
#include "EventFeatures.h"
#include "BluedDataPoint.h"


void onEventClassified(const EventFeatures &ev_features);


template <typename DataPointType>
void onEventDetected(std::shared_ptr<Event<DataPointType>> event) {
    program_log->info("detected an event at {}. Gave it the id {}", event->event_meta_data.event_time, event->event_meta_data.event_id);
}



template <typename DataPointType>
void storeEvent(std::shared_ptr<Event<DataPointType>> event) {
    program_log->info("storing event");

    static Hdf5EventStorage<DataPointType> storage;
    storage.storeEvent(*event);
}


template <typename DataPointType>
void connectEvents(SmartApp<DataPointType> &smart_app) {

    smart_app.getEventDetector()->event_detected.connect(0, &onEventDetected<DataPointType>);

    smart_app.getEventDetector()->event_detected.connect(1, &storeEvent<DataPointType>);

    smart_app.getDataClassifier()->event_classified.connect(&onEventClassified);
}
#endif //SMART_SCREEN_EVENTHANDLERS_H
