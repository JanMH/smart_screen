#ifndef SMART_SCREEN_SELECTPARTITIONS_H
#define SMART_SCREEN_SELECTPARTITIONS_H


std::map<EventMetaData::LabelType, std::vector<EventFeatures>>
putIntoBuckets(const std::vector<EventFeatures> &features);

std::vector<EventFeatures>
collectFromBuckets(const std::map<EventMetaData::LabelType, std::vector<EventFeatures>> &buckets);

std::vector<CrossValidationResult> crossValidate(EventLabelManager<BluedDataPoint> &labeled_events);

EventLabelManager<BluedDataPoint> initLabelManager(EventLabelManager<BluedDataPoint> labeled_events);

EventLabelManager<BluedDataPoint>
dropPartition(EventLabelManager<BluedDataPoint> labels, int total_number_of_partitions, int to_drop);

std::vector<EventFeatures>
getPartition(EventLabelManager<BluedDataPoint> labels, int total_number_of_partitions, int part_number);

CrossValidationResult
validatePartition(const EventLabelManager<BluedDataPoint> &labels, std::vector<EventFeatures> test_data);


EventLabelManager<BluedDataPoint>
dropPartition(EventLabelManager<BluedDataPoint> labels, int total_number_of_partitions, int to_drop) {
    assert(to_drop < total_number_of_partitions);
    unsigned long elements_per_partition = labels.labeled_events.size() / total_number_of_partitions;
    auto begin = labels.labeled_events.begin() + elements_per_partition * to_drop;
    unsigned long elements_to_drop_end = std::min(static_cast<unsigned long>(labels.labeled_events.size()), (to_drop + 1) * elements_per_partition);
    auto end = labels.labeled_events.begin() + elements_to_drop_end;
    auto r_end = labels.labeled_events.rbegin();
    if (to_drop != total_number_of_partitions - 1) {
        std::for_each(begin, end, [&labels, &r_end](EventFeatures &f) {
            std::swap(f, *r_end);
            ++r_end;
        });
    }
    labels.labeled_events.resize(labels.labeled_events.size() - elements_per_partition);
    return labels;
}

std::vector<EventFeatures>
getPartition(EventLabelManager<BluedDataPoint> labels, int total_number_of_partitions, int part_number) {

    assert(part_number < total_number_of_partitions);
    unsigned long elements_per_partition = labels.labeled_events.size() / total_number_of_partitions;
    auto begin = labels.labeled_events.begin() + elements_per_partition * part_number;
    unsigned long elements_to_fetch_end = std::min(static_cast<unsigned long>(labels.labeled_events.size()),
                                                   (part_number + 1) * elements_per_partition);
    auto end = labels.labeled_events.begin() + elements_to_fetch_end;
    return std::vector<EventFeatures>(begin, end);


}

std::map<EventMetaData::LabelType, std::vector<EventFeatures>>
putIntoBuckets(const std::vector<EventFeatures> &features) {
    std::map<EventMetaData::LabelType, std::vector<EventFeatures>> result;
    for (const auto &feature_vec:features) {
        result[*feature_vec.event_meta_data.label].push_back(feature_vec);
    }
    return result;
}

std::vector<EventFeatures>
collectFromBuckets(const std::map<EventMetaData::LabelType, std::vector<EventFeatures>> &buckets) {
    std::vector<EventFeatures> result;
    for (const auto &bucket: buckets) {
        result.insert(result.end(), bucket.second.begin(), bucket.second.end());
    }
    return result;
}


#endif //SMART_SCREEN_SELECTPARTITIONS_H
