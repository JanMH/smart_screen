#include <fstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "SerializeEventLabelManager.h"
#include <iostream>
#include <thread>

#include "PowerMetaData.h"
#include "AsyncDataQueue.h"
#include "BluedHdf5InputSource.h"

#include "DataClassifier.h"

#include "CrossValidationResult.h"
#include "SelectPartitions.h"


EventLabelManager<BluedDataPoint> initLabelManagerWithoutMisdetected(EventLabelManager<BluedDataPoint> labeled_events);

EventLabelManager<BluedDataPoint>
initLabelManagerWithoutSmallBuckets(EventLabelManager<BluedDataPoint> labeled_events);

void printNormalizedClassificationMatrix(EventLabelManager<BluedDataPoint> labels);


int main(int argc, char **argv) {

    using namespace std;


    if (argc < 3) {
        cout << "usage: event_detection_setup <label manager archive> <results archive>\n";
        return 0;
    }
    ifstream ifs(argv[1]);
    boost::archive::text_iarchive label_archive(ifs);
    EventLabelManager<BluedDataPoint> label_manager;
    label_archive >> label_manager;

    EventLabelManager<BluedDataPoint> labeled_events = initLabelManagerWithoutMisdetected(label_manager);
    printNormalizedClassificationMatrix(labeled_events);
    auto results = crossValidate(labeled_events);
    std::ofstream result_serialization_stream(argv[2]);

    boost::archive::text_oarchive ia(result_serialization_stream);
    ia << results;
}

void printNormalizedClassificationMatrix(EventLabelManager<BluedDataPoint> labels) {
    using namespace std;
    DataClassifier<BluedDataPoint> classifier;
    sort(labels.labeled_events.begin(), labels.labeled_events.end(), [](const EventFeatures &ev, const EventFeatures &ev2) {
        return *ev.event_meta_data.label < *ev2.event_meta_data.label;
    });
    classifier.setEventLabelManager(labels);

    std::vector<EventMetaData::LabelType> labels_only(labels.labeled_events.size());
    std::transform(labels.labeled_events.begin(), labels.labeled_events.end(), labels_only.begin(),[](const EventFeatures &ev) { return *ev.event_meta_data.label;});

    const static Eigen::IOFormat CSVFormat(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", "\n");
    cout << classifier.getNormalizedLabeledMatrix().format(CSVFormat) << "\n\n\n";

    for(const auto& label: labels_only) {
        cout << label << ",";
    }
    cout << "\n\n";

    for(const auto& ev_id: labels.labeled_events) {
        cout << ev_id.event_meta_data.event_id << ",";
    }
    cout << "\n\n";

}


std::vector<CrossValidationResult> crossValidate(EventLabelManager<BluedDataPoint> &labeled_events) {

    int number_of_partitions = 10;
    std::vector<CrossValidationResult> results;
    for (int i = 0; i < number_of_partitions; ++i) {
        EventLabelManager<BluedDataPoint> training_set = dropPartition(labeled_events, number_of_partitions, i);
        std::vector<EventFeatures> test_set = getPartition(labeled_events, number_of_partitions, i);
        results.push_back(validatePartition(labeled_events, test_set));

    }
    return results;

}

EventLabelManager<BluedDataPoint> initLabelManager(EventLabelManager<BluedDataPoint> labeled_events) {

    const EventMetaData::LabelType not_an_event = 666;
    unsigned long unlabeled_pos = labeled_events.labeled_events.size();
    labeled_events.labeled_events.insert(labeled_events.labeled_events.end(), labeled_events.unlabeled_events.begin(),
                                         labeled_events.unlabeled_events.end());
    std::for_each(labeled_events.labeled_events.begin() + unlabeled_pos, labeled_events.labeled_events.end(),
                  [not_an_event](EventFeatures &f) {
                      f.event_meta_data.label = not_an_event;
                  });
    std::random_shuffle(labeled_events.labeled_events.begin(), labeled_events.labeled_events.end());
    labeled_events.labels = decltype(labeled_events.labels)();
    labeled_events.unlabeled_events.clear();
    return labeled_events;
}

EventLabelManager<BluedDataPoint>
initLabelManagerWithoutMisdetected(EventLabelManager<BluedDataPoint> labeled_events) {
    labeled_events.unlabeled_events.clear();
    labeled_events.labels = decltype(labeled_events.labels)();
    std::random_shuffle(labeled_events.labeled_events.begin(), labeled_events.labeled_events.end());

    return labeled_events;
}


EventLabelManager<BluedDataPoint>
initLabelManagerWithoutSmallBuckets(EventLabelManager<BluedDataPoint> labeled_events) {
    labeled_events = initLabelManagerWithoutMisdetected(labeled_events);
    auto buckets = putIntoBuckets(labeled_events.labeled_events);
    const auto min_elements_in_bucket = 5;
    for (auto &bucket: buckets) {
        if (bucket.second.size() < min_elements_in_bucket) {
            std::cout << "clearing bucket " << bucket.first << " because it hasn't enough elements\n";
            bucket.second.clear();
        }
    }
    auto features = collectFromBuckets(buckets);
    labeled_events.labeled_events = features;
    std::random_shuffle(labeled_events.labeled_events.begin(), labeled_events.labeled_events.end());

    return labeled_events;
}


CrossValidationResult
validatePartition(const EventLabelManager<BluedDataPoint> &labels, std::vector<EventFeatures> test_data) {
    DataClassifier<BluedDataPoint> test_classifier;
    test_classifier.setEventLabelManager(labels);
    for (auto &event:test_data) {
        test_classifier.classifyOneEventAndPushToLabelManager(event);
    }
    std::vector<EventFeatures> classification_results = test_classifier.getEventLabelManager().unlabeled_events;
    assert(classification_results.size() == test_data.size());
    CrossValidationResult cvs;

    for (decltype(classification_results.size()) i = 0; i < classification_results.size(); ++i) {
        Guess g(*test_data[i].event_meta_data.label, *classification_results[i].event_meta_data.label, *classification_results[i].event_meta_data.event_id);
        cvs.broken_down.push_back(g);

    }
    return cvs;

}
