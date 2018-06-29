#ifndef BLUEDHDF5CONVERTER_H
#define BLUEDHDF5CONVERTER_H

#include <string>
#include "H5Cpp.h"
#include "AsyncDataQueue.h"
#include "BluedInputSource.h"

class BluedHdf5Converter
{
public:
    void convertToHdf5(const std::string& input_file, const std::string& output_file);
private:
    void createNewDataSpace(const std::string& output_file);
    void writeData(BluedDataManager& mgr);
private:
    static const int rank = 2;
    static const int ncols = 4;
    static const int chunk_size = 240;
    hsize_t dims[rank] = {0, ncols};

    
    H5::H5File file;

    H5::DataSpace file_data_space;
    H5::DataSet dataset;
    
};

#endif // BLUEDHDF5CONVERTER_H
