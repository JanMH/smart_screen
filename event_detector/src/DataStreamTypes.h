
#ifndef SMART_SCREEN_DATASTREAMTYPEDEFINITIONS_H
#define SMART_SCREEN_DATASTREAMTYPEDEFINITIONS_H


#include <deque>

template <typename DataPointType>
class DataStreamTypes {
public:
    typedef typename std::deque<DataPointType>::iterator DataStreamIterator;

};

#endif //SMART_SCREEN_DATASTREAMTYPEDEFINITIONS_H
