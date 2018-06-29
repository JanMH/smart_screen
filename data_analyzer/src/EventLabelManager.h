#ifndef SMART_SCREEN_LABELEDEVENTS_H
#define SMART_SCREEN_LABELEDEVENTS_H


#include <map>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <algorithm>
#include <boost/optional.hpp>
#include <set>

#include "EventFeatures.h"
#include "Logger.h"

struct DefaultDataPoint;

struct LabelTimePair {
    EventMetaData::LabelType label;
    EventMetaData::TimeType time;
};
namespace __detail {
    struct TimePairComparator {
        static bool compareLabels(const LabelTimePair &l1, const LabelTimePair &l2) {
            const EventMetaData::MSDurationType accepted_time_difference(2500);
            return l1.time + accepted_time_difference < l2.time;
        }

        bool operator()(const LabelTimePair &l1, const LabelTimePair &l2) const {
            return compareLabels(l1, l2);
        };
    };
}
/**
 * @brief This class provides convenience functions to managing all labeled and unlabeled events.
 *
 * It may also be serialized or deserialized in whole because it contains the relevant data for our trained model.
 * @tparam DataPointType The data point type. This is most likely the DefaultDataPoint
 * @note None of the methods of this function are thread safe.
 */
template<typename DataPointType = DefaultDataPoint> class EventLabelManager {

public:


    /**
     * Searches our already preinitialized event time list for a matching time and adds the label to this event.
     *
     * If one is found the event is marked as labeled and added to the labeled events. This function is used for prerecorded data sets only.
     * @param event
     * @return
     */
    bool findLabelAndAddEvent(const EventFeatures &event);

    void addUnLabeledEvent(const EventFeatures &event);


    void addLabeledEvent(const EventFeatures &event, EventMetaData::LabelType label);

    void addClassifiedEvent(const EventFeatures &event);

    void addClassifiedEvent(const EventFeatures &event, EventMetaData::LabelType label);


    bool addLabel(const LabelTimePair labels);

    bool addLabel(EventMetaData::EventIdType event_id, EventMetaData::LabelType label);

    /**
     * This function searches the list of label time pairs and returns the label type if a matching label time pair is found
     * @param event
     * @return
     */
    boost::optional<EventMetaData::LabelType> getEventLabel(const EventFeatures &event) const;

    bool labelIsForEvent(const LabelTimePair &label_time, const EventFeatures &event) const;

    void loadLabelsFromFile(std::string file_name);


    boost::optional<EventFeatures> getEvent(EventMetaData::EventIdType event_id);

    void setEvent(EventMetaData::EventIdType event_id, EventFeatures event);

    std::vector<EventFeatures> labeled_events; ///< All events that were explicitly labeled by the user
    std::vector<EventFeatures> unlabeled_events; ///< All unlabeled or simply classified events.
    std::set<LabelTimePair, __detail::TimePairComparator> labels; ///< A list of lables identified by time points. This is only relevant for prerecorded data sets.

private:


};

template<typename DataPointType> bool
EventLabelManager<DataPointType>::findLabelAndAddEvent(const EventFeatures &event) {
    boost::optional<EventMetaData::LabelType> opt_label = boost::none;
    if (event.event_meta_data.label == boost::none) {
        opt_label = this->getEventLabel(event);
    }

    if (opt_label != boost::none) {
        addLabeledEvent(event, *opt_label);
        return true;
    }
    return false;
}

template<typename DataPointType> void
EventLabelManager<DataPointType>::addLabeledEvent(const EventFeatures &event, EventMetaData::LabelType label) {
    this->labeled_events.push_back(event);
    this->labeled_events.back().event_meta_data.label = label;
}

template<typename DataPointType> void EventLabelManager<DataPointType>::addClassifiedEvent(const EventFeatures &event) {
    this->unlabeled_events.push_back(event);
}

template<typename DataPointType> boost::optional<EventMetaData::LabelType>
EventLabelManager<DataPointType>::getEventLabel(const EventFeatures &event) const {
    if (labels.empty()) {
        program_log->debug("No labels loaded! Cannot infer the label from the label-time list");
        return boost::none;
    }
    LabelTimePair to_search;
    to_search.time = event.event_meta_data.event_time;
    auto iter = labels.find(to_search);
    if (iter != this->labels.end()) {
        return iter->label;
    }

    program_log->debug("found no label in the label-time list");
    iter = labels.lower_bound(to_search);
    if (iter != labels.end()) {

        program_log->debug("closest match after event occurrence is: {}", iter->time);
    }
    if (iter != labels.begin()) {
        --iter;

        program_log->debug("closest match before event occurrence is: {}", iter->time);
    }


    return boost::none;
}

template<typename DataPointType> void EventLabelManager<DataPointType>::loadLabelsFromFile(std::string file_name) {
    std::ifstream file(file_name);
    if (!file.good()) {
        std::cerr << "Failed to open file: " + file_name + "\n";
        return;
    }
    while (true) {
        LabelTimePair l;
        long epoch;
        file >> epoch;
        l.time = boost::posix_time::from_time_t(epoch);
        file.ignore(10, ',');
        file >> l.label;
        file.ignore(50, '\n');
        if (file.good()) {
            labels.insert(l);
        } else {
            break;
        }
    }
}

template<typename DataPointType> bool
EventLabelManager<DataPointType>::labelIsForEvent(const LabelTimePair &label_time, const EventFeatures &event) const {
    LabelTimePair to_search;
    to_search.time = event.event_meta_data.event_time;
    return !__detail::TimePairComparator::compareLabels(to_search, label_time) &&
           !__detail::TimePairComparator::compareLabels(label_time, to_search);
}

template<typename DataPointType> bool EventLabelManager<DataPointType>::addLabel(const LabelTimePair labels) {
    for (int i = 0; i < unlabeled_events.size(); ++i) {
        if (labelIsForEvent(labels, unlabeled_events[i])) {
            if (i != unlabeled_events.size() - 1) {
                std::swap(this->unlabeled_events.back(), this->unlabeled_events[i]);
            }
            this->addLabeledEvent(this->unlabeled_events.back(), labels.label);
            this->unlabeled_events.pop_back();
            return true;
        }
    }
    return false;
}

template<typename DataPointType>
bool EventLabelManager<DataPointType>::addLabel(EventMetaData::EventIdType event_id, EventMetaData::LabelType label) {
    auto iter = std::find_if(this->unlabeled_events.begin(), this->unlabeled_events.end(),
                             [event_id](const EventFeatures &event) {
                                 return event.event_meta_data.event_id == event_id;
                             });
    if (iter != this->unlabeled_events.end()) {
        std::swap(this->unlabeled_events.back(), *iter);
        this->addLabeledEvent(this->unlabeled_events.back(), label);
        this->unlabeled_events.pop_back();
        return true;
    }
    auto iter2 = std::find_if(this->labeled_events.begin(), this->labeled_events.end(),
                              [event_id](const EventFeatures &event) {
                                  return event.event_meta_data.event_id == event_id;
                              });

    if (iter2 != this->labeled_events.end()) {
        program_log->warn("The event with id {} already had the label {}. Overwriting it's label with {}", event_id,
                          iter2->event_meta_data.label, label);
        iter2->event_meta_data.label = label;
        return true;
    }
    return false;
}


template<typename DataPointType> void
EventLabelManager<DataPointType>::addClassifiedEvent(const EventFeatures &event, EventMetaData::LabelType label) {
    this->unlabeled_events.push_back(event);
    this->unlabeled_events.back().event_meta_data.label = label;
}

template<typename DataPointType>
void EventLabelManager<DataPointType>::addUnLabeledEvent(const EventFeatures &event) {
    this->unlabeled_events.push_back(event);
}

template<typename DataPointType> boost::optional<EventFeatures>
EventLabelManager<DataPointType>::getEvent(EventMetaData::EventIdType event_id) {
    auto iter = std::find_if(this->unlabeled_events.begin(), this->unlabeled_events.end(),
                             [event_id](const EventFeatures &event) {
                                 return event.event_meta_data.event_id == event_id;
                             });
    if (iter != this->unlabeled_events.end()) {

        return *iter;
    }
    return boost::none;

}

template<typename DataPointType> void
EventLabelManager<DataPointType>::setEvent(EventMetaData::EventIdType event_id, EventFeatures event) {
    auto iter = std::find_if(this->unlabeled_events.begin(), this->unlabeled_events.end(),
                             [event_id](const EventFeatures &ev) {
                                 return ev.event_meta_data.event_id == event_id;
                             });
    if (iter != this->unlabeled_events.end()) {
        *iter = event;
    }
}


#endif //SMART_SCREEN_LABELEDEVENTS_H
