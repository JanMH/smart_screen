#ifndef SMART_SCREEN_UTILITIES_H_H
#define SMART_SCREEN_UTILITIES_H_H

#include <boost/iterator/transform_iterator.hpp>

namespace __detail {
    template<typename DataPointType> auto getVoltage(const DataPointType &dp) -> decltype(dp.voltage()) {
        return dp.voltage();
    }
    template<typename DataPointType> auto getAmpere(const DataPointType &dp) -> decltype(dp.ampere()) {
        return dp.ampere();
    }
}

template<typename IterType> auto makeVoltageIterator(
        IterType iter) -> boost::transform_iterator<decltype(&__detail::getVoltage<decltype(*iter)>), IterType> {
        return boost::make_transform_iterator(iter, &__detail::getVoltage<decltype(*iter)>);
}

template<typename IterType> auto makeAmpereIterator(
        IterType iter) -> boost::transform_iterator<decltype(&__detail::getAmpere<decltype(*iter)>), IterType> {
    return boost::make_transform_iterator(iter, &__detail::getAmpere<decltype(*iter)>);
}

#endif //SMART_SCREEN_UTILITIES_H_H
