
#ifndef SMART_SCREEN_HDF5MUTEX_H
#define SMART_SCREEN_HDF5MUTEX_H

#include <mutex>


extern std::mutex hdf5_mutex; ///< This mutex protects the hdf5 library since it can not be used in multiple threads at the same time (even for entirely separate objects and files) due to internal buffers

#endif //SMART_SCREEN_HDF5MUTEX_H
