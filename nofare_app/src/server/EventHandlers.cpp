
#include "EventHandlers.h"
#include "Logger.h"



void onEventClassified(const EventFeatures &ev_features) {
    program_log->info("classified an event id: {}, label: {}, certainty: {}", ev_features.event_meta_data.event_id,
                      *ev_features.event_meta_data.label,
                      ev_features.event_meta_data.classification_certainty
    );
}
