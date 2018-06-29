#ifndef SMART_SCREEN_DATAANALYZER_H
#define SMART_SCREEN_DATAANALYZER_H

#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>

#include <nabo/nabo.h>
#include <boost/optional/optional_io.hpp>
#include <boost/signals2/signal.hpp>

#include "EventFeatures.h"
#include "Algorithms.h"
#include "EventLabelManager.h"
#include "ClassificationConfig.h"
#include "FeatureExtractor.h"

#include "Logger.h"


struct DefaultDataPoint;

/**
 * @brief The data classifier is the core of event classification.
 *
 * It classifies events in a different thread and provides thread safe functions to add events to the queue, add labels and copy and set the model of labeled feature vectors.
 * @tparam DataPointType
 */
template<typename DataPointType = DefaultDataPoint> class DataClassifier {
public:

    typedef float DataFeatureType;

    /**
     * @brief Starts an extra thread that classifies incoming events one at a time.
     * @param label_location Labels are loaded from this file.
     * @param config parameters incluencing the classification process.
     */
    void
    startClassification(const std::string &label_location, const ClassificationConfig &config = ClassificationConfig());

    /**
     * @brief Starts an extra thread that classifies incoming events one at a time.
     * @param config parameters incluencing the classification process.
     */
    void startClassification(const ClassificationConfig &config = ClassificationConfig());

    /**
     * Stops the analyzation thread immediately possibly discarding events that have already been detected but havn't been analyzed yet. If possible please use stopAnalyzingWhenDone instead.
     */
    void stopAnalyzingImmediately();

    /**
     * Stops the analyzation thread as soon as there are no more events on the stack.
     */
    void stopAnalyzingWhenDone();


    /**
     * Adds and event to the queue. The event is copied in the process.
     * @param e
     */
    void pushEvent(const Event<DataPointType> &e);

    /**
     * Adds and event to the queue. The event is moved in the process.
     * @param e
     */
    void pushEvent(Event<DataPointType> &&e);

    /**
     * @warning This method is not thread safe and should only be used if startClassification was *not* called beforehand.
     * @param e An already extracted feature vector
     */
    void classifyOneEventAndPushToLabelManager(EventFeatures e);

    EventFeatures classifyOneEvent(EventFeatures e);

    /**
     * Adds a label identified by the event's time of occurence
     * @param label A label time pair.
     */
    void addLabel(const LabelTimePair &label);

    /**
     * Add's a label identiefied by the event's id.
     * @param event_id The id of the event
     * @param label  The label of the event
     * @returns true if an event with this id was found. False otherwise.
     */
    bool addLabelById(EventMetaData::EventIdType event_id, EventMetaData::LabelType label);


    /**
     * Joins the thread.
     */
    void join() {
        if (this->runner.joinable())
            this->runner.join();
    }

    ~DataClassifier() { this->stopAnalyzingImmediately(); }

    /**
     * Returns the raw feature vector matrix. Used mainly for debugging pourpouses.
     * @return An eigenlib Matrix
     */
    Eigen::MatrixXf getUnnormalizedLabeledMatrix();

    /**
     * Returns a normalized matrix.
     * @return An eigenlib matrix
     */
    Eigen::MatrixXf getNormalizedLabeledMatrix();

    /**
     *
     * @returns The EventLabelManager that contains all labeled and unlabeled feature vectors.
     */
    EventLabelManager<DataPointType> getEventLabelManager();

    /**
     *
     * @param label_manager The EventLabelManager that contains all labeled and unlabeled feature vectors.
     */
    void setEventLabelManager(EventLabelManager<DataPointType> label_manager);

    /**
     * @return the numbers of elements that still need to be processed.
     */
    std::size_t getNumberOfElementsOnStack();

    void reclassify(EventMetaData::EventIdType event_id);

private:
    void run();

    void processOneEvent(const Event<DataPointType> &features);

    Eigen::VectorXf convertToEigenVector(const EventFeatures &features);

    void regenerateMatrix();

    void addEventToNormalizedMatrix(const EventFeatures &features);

    bool needToRegenerateMatrixForVector(const Eigen::VectorXf &vec);

    bool featureVectorIsInRange(const Eigen::VectorXf &vec);

    void generateNormalizationVectors(const Eigen::MatrixXf &matrix);

    void generateRescaleNormalizationVectors(const Eigen::MatrixXf &matrix);

    void generateStandardizeNormalizationVectors(const Eigen::MatrixXf &matrix);

    void generateFirstNormalizationVectors(const Eigen::MatrixXf &matrix);

    Eigen::MatrixXf generateMatrixFromLabeledEvents();

    std::vector<DataFeatureType> eigenToStdVector(const Eigen::VectorXf &vec) {
        std::vector<DataClassifier::DataFeatureType> result;
        result.reserve(vec.size());
        for (int i = 0; i < vec.size(); ++i) {
            result.push_back(vec(i));
        }
        return result;

    }


    Eigen::VectorXf generateMaxVector(const Eigen::MatrixXf &matrix);

    Eigen::VectorXf generateMinVector(const Eigen::MatrixXf &matrix);

    void pushToMatrix(const Eigen::VectorXf &vec);

    Eigen::VectorXf normalizeEvent(Eigen::VectorXf vec);

    void normalizeMatrix(Eigen::MatrixXf &matrix);


public: // Attributes

    boost::signals2::signal<void(const EventFeatures &)> event_classified;

private:
    EventLabelManager<DataPointType> event_label_manager;
    ClassificationConfig classification_config;
    FeatureExtractor feature_extractor;
    std::thread runner;
    bool continue_analyzing = true;
    std::mutex events_mutex;
    std::condition_variable events_empty_variable;
    std::vector<Event<DataPointType>> events;
    Eigen::MatrixXf labeled_matrix;
    Eigen::VectorXf normalization_mul_vector;
    Eigen::VectorXf normalization_add_vector;
    std::unique_ptr<Nabo::NNSearchF> nns;


};


template<typename DataPointType> void DataClassifier<DataPointType>::pushEvent(const Event<DataPointType> &e) {
    {
        std::unique_lock<std::mutex> events_lock(events_mutex);
        events.push_back(e);
    }
    events_empty_variable.notify_one();
}

template<typename DataPointType> void DataClassifier<DataPointType>::pushEvent(Event<DataPointType> &&e) {
    {
        std::unique_lock<std::mutex> events_lock(events_mutex);
        events.push_back(std::move(e));
    }
    events_empty_variable.notify_one();
}

template<typename DataPointType> void
DataClassifier<DataPointType>::processOneEvent(const Event<DataPointType> &event) {
    EventFeatures features = feature_extractor.extractFeatures(event);


    if (event_label_manager.findLabelAndAddEvent(features)) {
        this->addEventToNormalizedMatrix(features);
    } else {
        this->classifyOneEventAndPushToLabelManager(features);
    }
}

template<typename DataPointType> void DataClassifier<DataPointType>::classifyOneEventAndPushToLabelManager(
        EventFeatures features) {

    try {
        auto labeled_event = this->classifyOneEvent(features);
    } catch (...) {
        features.event_meta_data.label = EventMetaData::EventUnknown;

        this->event_label_manager.addUnLabeledEvent(features);

    }

    this->event_classified(features);
    this->event_label_manager.addClassifiedEvent(std::move(features));
}

template<typename DataPointType> EventFeatures DataClassifier<DataPointType>::classifyOneEvent(
        EventFeatures features) {
    const unsigned long k = 5;
    if (this->event_label_manager.labeled_events.size() <= k) {
        program_log->info(
                "not enough elements in cloud yet to classify the event. Current number of elements: {}. {} are needed",
                this->event_label_manager.labeled_events.size(),
                k
        );


        throw -1;
    }
    using namespace Eigen;
    using namespace Nabo;
    program_log->debug("Classifying event:");

    VectorXi indices;
    VectorXf dists2;

    // Look for the 5 nearest neighbours of each query point,
    // We do not want approximations but we want to sort by the distance,
    indices.resize(k);
    dists2.resize(k);
    auto vect = this->convertToEigenVector(features);
    nns->knn(normalizeEvent(vect), indices, dists2, k);
    program_log->debug("The labels of the closest neighbors are: ");

    std::map<EventMetaData::LabelType, int> labels;
    for (int i = 0; i < k; ++i) {
        labels[*event_label_manager.labeled_events[indices(i)].event_meta_data.label]++;
        program_log->debug("{}", event_label_manager.labeled_events[indices(i)].event_meta_data.label);
    }
    int max = -1;
    EventMetaData::LabelType max_label;
    for (auto &x: labels) {
        if (x.second > max) {
            max = x.second;
            max_label = x.first;
        }
    }
    features.event_meta_data.label = max_label;
    features.event_meta_data.classification_certainty = max / static_cast<float>(k);
    return features;
}

template<typename DataPointType> void
DataClassifier<DataPointType>::startClassification(const std::string &label_location,
                                                   const ClassificationConfig &config) {
    this->join();
    this->event_label_manager.loadLabelsFromFile(label_location);
    this->startClassification(config);
}

template<typename DataPointType> void DataClassifier<DataPointType>::stopAnalyzingImmediately() {
    this->continue_analyzing = false;
    this->events_empty_variable.notify_all();
    this->join();

}

template<typename DataPointType> void DataClassifier<DataPointType>::stopAnalyzingWhenDone() {
    {
        std::unique_lock<std::mutex> events_lock(this->events_mutex);
        this->events_empty_variable.wait(events_lock, [this]() {
            return this->events.empty();
        });
    }

    this->stopAnalyzingImmediately();

}

template<typename DataPointType> Eigen::VectorXf
DataClassifier<DataPointType>::convertToEigenVector(const EventFeatures &features) {
    Eigen::VectorXf result;
    result.resize(features.feature_vector.size());
    int count = 0;
    for (auto &f:features.feature_vector) {
        result(count) = f;
        ++count;
    }

    return result;
}

template<typename DataPointType> void
DataClassifier<DataPointType>::addEventToNormalizedMatrix(const EventFeatures &features) {


    Eigen::VectorXf feature_vec = convertToEigenVector(features);
    if (needToRegenerateMatrixForVector(feature_vec)) {

        regenerateMatrix();

    } else {
        pushToMatrix(normalizeEvent(feature_vec));
    }


}

template<typename DataPointType> bool
DataClassifier<DataPointType>::featureVectorIsInRange(const Eigen::VectorXf &vec) {
    if (this->normalization_add_vector.size() <= 0) {
        return false;
    }

    Eigen::VectorXf v = this->normalizeEvent(Eigen::VectorXf(vec));
    for (int i = 0; i < v.size(); ++i) {
        if (v(i) < -1.0 || v(i) > 1.0) {
            return false;
        }
    }

    return true;
}

template<typename DataPointType> void
DataClassifier<DataPointType>::generateNormalizationVectors(const Eigen::MatrixXf &matrix) {
    if (this->event_label_manager.labeled_events.size() == 1) {
        generateFirstNormalizationVectors(matrix);
    } else if (classification_config.normalization_mode == NormalizationMode::Rescale) {
        generateRescaleNormalizationVectors(matrix);
    } else {
        generateStandardizeNormalizationVectors(matrix);
    }

}

template<typename DataPointType> void
DataClassifier<DataPointType>::generateRescaleNormalizationVectors(const Eigen::MatrixXf &matrix) {

    Eigen::VectorXf max_vector;
    Eigen::VectorXf min_vector;
    min_vector = generateMinVector(matrix);
    max_vector = generateMaxVector(matrix);


    for (long i = 0; i < normalization_mul_vector.size(); ++i) {
        normalization_mul_vector(i) = 2.0f / (max_vector(i) - min_vector(i));
    }
    for (long i = 0; i < normalization_mul_vector.size(); ++i) {
        this->normalization_add_vector(i) = -1.0f * min_vector(i) * this->normalization_mul_vector(i) - 1.0f;
    }

}

template<typename DataPointType> void
DataClassifier<DataPointType>::generateStandardizeNormalizationVectors(const Eigen::MatrixXf &matrix) {
    if (matrix.cols() < 2) {
        return;
    }

    normalization_mul_vector.resize(matrix.rows());
    normalization_add_vector.resize(matrix.rows());
    const float *data = matrix.row(0).data();
    const float min_deviation = 0.001;


    for (long i = 0; i < matrix.rows(); ++i) {
        auto row_vector = eigenToStdVector(matrix.row(i));

        FeatureExtractor::FeatureType mean = Algorithms::mean(row_vector.begin(), row_vector.end());


        FeatureExtractor::FeatureType variance = Algorithms::variance(row_vector.begin(), row_vector.end());

        FeatureExtractor::FeatureType deviation = std::max(min_deviation, std::sqrt(variance));

        normalization_mul_vector(i) = 1.f / deviation;
        normalization_add_vector(i) = -mean / deviation;
    }
}

template<typename DataPointType> Eigen::VectorXf
DataClassifier<DataPointType>::generateMaxVector(const Eigen::MatrixXf &matrix) {
    Eigen::VectorXf max = matrix.col(0);
    for (long i = 1; i < matrix.cols(); ++i) {
        for (long j = 0; j < matrix.rows(); ++j) {
            if (matrix(j, i) > max(j)) {
                max(j) = matrix(j, i);
            }
        }
    }
    return max;
}


template<typename DataPointType> Eigen::VectorXf
DataClassifier<DataPointType>::generateMinVector(const Eigen::MatrixXf &matrix) {
    Eigen::VectorXf min = matrix.col(0);
    for (long i = 1; i < matrix.cols(); ++i) {
        for (long j = 0; j < matrix.rows(); ++j) {
            if (matrix(j, i) < min(j)) {
                min(j) = matrix(j, i);
            }
        }
    }
    return min;
}

template<typename DataPointType> void DataClassifier<DataPointType>::pushToMatrix(const Eigen::VectorXf &vec) {
    long rows = vec.size();
    labeled_matrix.resize(rows, labeled_matrix.cols() + 1);
    labeled_matrix.col(labeled_matrix.cols() - 1) = vec;
}

template<typename DataPointType> void DataClassifier<DataPointType>::regenerateMatrix() {
    if (event_label_manager.labeled_events.empty()) {
        return;
    }
    this->labeled_matrix = generateMatrixFromLabeledEvents();

    this->generateNormalizationVectors(this->labeled_matrix);
    this->normalizeMatrix(this->labeled_matrix);
    nns = std::unique_ptr<Nabo::NNSearchF>(Nabo::NNSearchF::createKDTreeTreeHeap(this->labeled_matrix));
}

template<typename DataPointType> void DataClassifier<DataPointType>::normalizeMatrix(Eigen::MatrixXf &matrix) {
    const static Eigen::IOFormat CSVFormat(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", "\n");

    for (long i = 0; i < matrix.cols(); ++i) {
        matrix.col(i) = normalizeEvent(matrix.col(i));
    }

}

template<typename DataPointType> Eigen::VectorXf DataClassifier<DataPointType>::normalizeEvent(Eigen::VectorXf vec) {

    for (long j = 0; j < vec.size(); ++j) {
        vec(j) = vec(j) * this->normalization_mul_vector(j) + this->normalization_add_vector(j);
    }
    return vec;
}

template<typename DataPointType> bool
DataClassifier<DataPointType>::needToRegenerateMatrixForVector(const Eigen::VectorXf &vec) {
    return this->classification_config.normalization_mode != NormalizationMode::Rescale ||
           !featureVectorIsInRange(vec) || this->event_label_manager.labeled_events.size() <= 1;
}

template<typename DataPointType> void
DataClassifier<DataPointType>::generateFirstNormalizationVectors(const Eigen::MatrixXf &matrix) {
    normalization_mul_vector = matrix.col(0);
    normalization_mul_vector.setConstant(1.0);
    normalization_add_vector = -matrix.col(0);
}

template<typename DataPointType> void DataClassifier<DataPointType>::addLabel(const LabelTimePair &label) {
    std::lock_guard<std::mutex> lock(this->events_mutex);
    this->event_label_manager.addLabel(label);


}

template<typename DataPointType> bool
DataClassifier<DataPointType>::addLabelById(EventMetaData::EventIdType event_id, EventMetaData::LabelType label) {
    std::lock_guard<std::mutex> lock(this->events_mutex);
    if (this->event_label_manager.addLabel(event_id, label)) {
        this->regenerateMatrix();
        return true;
    } else {
        return false;
    }
}

template<typename DataPointType> void
DataClassifier<DataPointType>::startClassification(const ClassificationConfig &config) {

    this->continue_analyzing = true;
    feature_extractor.setConfig(config);
    runner = std::thread(&DataClassifier<DataPointType>::run, this);
}

template<typename DataPointType> void DataClassifier<DataPointType>::run() {

    while (true) {
        // wait until an event is pushed
        std::unique_lock<std::mutex> events_lock(this->events_mutex);
        this->events_empty_variable.wait(events_lock, [this]() {
            return !this->events.empty() || !this->continue_analyzing;
        });
        // if we dont want to analyze events anymore, quit
        if (!this->continue_analyzing) {
            break;
        }
        this->processOneEvent(this->events.back());
        this->events.pop_back();
    }
}

template<typename DataPointType> Eigen::MatrixXf DataClassifier<DataPointType>::getNormalizedLabeledMatrix() {
    return this->labeled_matrix;
}

template<typename DataPointType> EventLabelManager<DataPointType>
DataClassifier<DataPointType>::getEventLabelManager() {

    std::lock_guard<std::mutex> events_lock(this->events_mutex);
    auto x = this->event_label_manager;
    return x;
}

template<typename DataPointType> void
DataClassifier<DataPointType>::setEventLabelManager(EventLabelManager<DataPointType> label_manager) {
    std::lock_guard<std::mutex> l(events_mutex);
    this->event_label_manager = label_manager;
    regenerateMatrix();
}

template<typename DataPointType> std::size_t DataClassifier<DataPointType>::getNumberOfElementsOnStack() {
    std::lock_guard<std::mutex> l(events_mutex);

    return events.size();
}

template<typename DataPointType> Eigen::MatrixXf DataClassifier<DataPointType>::getUnnormalizedLabeledMatrix() {
    std::lock_guard<std::mutex> l(events_mutex);

    return generateMatrixFromLabeledEvents();
}

template<typename DataPointType> Eigen::MatrixXf DataClassifier<DataPointType>::generateMatrixFromLabeledEvents() {
    Eigen::MatrixXf result;

    long rows = event_label_manager.labeled_events.back().feature_vector.size();
    long cols = event_label_manager.labeled_events.size();

    result.resize(rows, cols);
    for (int i = 0; i < cols; ++i) {
        Eigen::VectorXf vec = convertToEigenVector(this->event_label_manager.labeled_events[i]);
        result.col(i) = vec;
    }
    return result;
}


template<typename DataPointType> void DataClassifier<DataPointType>::reclassify(EventMetaData::EventIdType event_id) {
    std::lock_guard<std::mutex> l(events_mutex);
    auto event = this->event_label_manager.getEvent(event_id);
    if (event.is_initialized()) {
        auto classified = this->classifyOneEvent(*event);
        this->event_label_manager.setEvent(event_id, classified);
    } else {
        program_log->warn("Could not reclassify the event {}. No such event.", event_id);
    }
}


#endif //SMART_SCREEN_DATAANALYZER_H




























