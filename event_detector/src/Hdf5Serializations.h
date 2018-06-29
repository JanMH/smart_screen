
#ifndef SMART_SCREEN_HDF5SERIALIZATIONS_H
#define SMART_SCREEN_HDF5SERIALIZATIONS_H


#include <Logger.h>
#include "Event.h"
#include "H5Cpp.h"

/*
 * The following functions and structs provide a generic way to store data within an hdf5 file.
 *
 *
 */

namespace hdf5_serializations {


    /**
     * This class creates a two dimensional data set within a file.
     * @param file The file in which the data set will be created
     * @param data_type  The type of data within the set. For instance  H5::DataType(H5::PredType::NATIVE_FLOAT)
     * @param data_width The width of the data set matrix
     * @param data_height The height of the data set matrix
     * @param data_set_name  The name of the data set
     * @return A data set object with which one can modify data within the data set
     */
    inline H5::DataSet createDataSetInFile(H5::H5File &file, const H5::DataType &data_type, hsize_t data_width,
                                           hsize_t data_height,
                                           const std::string &data_set_name) {
        using namespace H5;
        const int rank = 2;
        const hsize_t chunk_size = 240;
        hsize_t dims[rank] = {data_height, data_width};

        hsize_t max_dims[rank] = {data_height, data_width};


        auto file_data_space = DataSpace(rank, dims, max_dims);
        DSetCreatPropList cparms;
        hsize_t chunk_dims[rank] = {chunk_size, 2};
        cparms.setChunk(rank, chunk_dims);

        return file.createDataSet(data_set_name, data_type, file_data_space, cparms);
    }

    /**
     * Same as above
     * @see createDataSetInFile
     */
    inline H5::DataSet createDataSetInFile(H5::H5File &file, const H5::DataType &data_type, hsize_t data_width,
                                           hsize_t data_height) {
        return createDataSetInFile(file, data_type, data_width, data_height, "data");
    }


    /**
     * A struct to allow us to perform partial template deduction
     * @tparam DataPointType The type of data point we will use. For instance a DefaultDataPoint
     * @tparam IteratorType The type of iterator we will use. This should probably be deduced
     */
    template<typename DataPointType, typename IteratorType> struct Serializer { // general template
        static void serialize(H5::H5File &hdf_file, IteratorType begin, IteratorType end);
    };

    /**
     * A concrete serializer for default data points
     * @tparam IteratorType The type of iterator. This will probably be deduced
     */
    template<typename IteratorType> struct Serializer<DefaultDataPoint, IteratorType> { // Specialization for DefaultDataPoints
        static void serialize(H5::H5File &hdf_file, IteratorType begin, IteratorType end) {
            program_log->debug("serializing default data points");
            auto num_data_points = end - begin;
            auto data_set = createDataSetInFile(hdf_file, H5::DataType(H5::PredType::NATIVE_FLOAT), 2, num_data_points);
            std::vector<float> raw_data;
            raw_data.resize(num_data_points * 2);

            for (unsigned j = 0; j < raw_data.size(); ++j) {
                raw_data[j] = begin->voltage();
                ++j;
                raw_data[j] = begin->ampere();
                ++begin;
            }

            data_set.write(reinterpret_cast<const void *>(raw_data.data()), H5::PredType::NATIVE_FLOAT);
        }
    };

    /**
     * A serializer for a blued data point
     * @tparam IteratorType
     */
    template<typename IteratorType> struct Serializer<BluedDataPoint, IteratorType> { // Specialization for DefaultDataPoints
        static void serialize(H5::H5File &hdf_file, IteratorType begin, IteratorType end) {
            program_log->debug("serializing blued data points");
            auto num_data_points = end - begin;
            auto data_set = createDataSetInFile(hdf_file, H5::DataType(H5::PredType::NATIVE_FLOAT), 4, num_data_points);
            std::vector<float> raw_data;


            while(begin != end) {
                raw_data.push_back(begin->x_value);
                raw_data.push_back(begin->current_a);
                raw_data.push_back(begin->current_b);
                raw_data.push_back(begin->voltage_a);
                ++begin;
            }

            data_set.write(reinterpret_cast<const void *>(raw_data.data()), H5::PredType::NATIVE_FLOAT);
        }
    };

    /**
     * This function serializes an event. It automatically detects the data point type based on the type that is returned when dereferencing the IteratorType
     * @tparam IteratorType The type of an iterator. This will usually deduced automatically
     * @param hdf_file The file into which the data will be written
     * @param begin The beginning of event data. For instance Event<DataPointType>::event_data.begin()
     * @param end The end of event data.For instance. Event<DataPointType>::event_data.end()
     */
    template<typename IteratorType> void serializeEvent(H5::H5File &hdf_file, IteratorType begin, IteratorType end) {
        typedef typename std::remove_reference<decltype(*begin)>::type DataPointTypeNoRef;
        typedef typename std::remove_cv<DataPointTypeNoRef>::type DataPointType;


        hdf5_serializations::Serializer<DataPointType, IteratorType>::serialize(
                hdf_file,
                begin,
                end
        );
    }


}
#endif //SMART_SCREEN_HDF5SERIALIZATIONS_H
