#ifndef SMART_SCREEN_CROSSVALIDATIONSUCCESS_H
#define SMART_SCREEN_CROSSVALIDATIONSUCCESS_H

#include <EventMetaData.h>
#include <DefaultDataPoint.h>
#include <tuple>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/serialization.hpp>

struct Guess {
    EventMetaData::LabelType actual_label;
    EventMetaData::LabelType classified_label;
    unsigned long event_id;

    Guess() = default;

    Guess(const Guess &) = default;

    Guess(EventMetaData::LabelType actual, EventMetaData::LabelType classiefied, unsigned long id) : actual_label(
            actual), classified_label(classiefied), event_id(id) {}
    template<class Archive> void serialize(Archive &ar, const unsigned int version) {
        ar & actual_label;
        ar & classified_label;
        ar & event_id;
    }
};

struct CrossValidationResult {
    std::vector<Guess> broken_down;

    template<class Archive> void serialize(Archive &ar, const unsigned int version) {
        ar & broken_down;
    }
};

#endif //SMART_SCREEN_CROSSVALIDATIONSUCCESS_H
