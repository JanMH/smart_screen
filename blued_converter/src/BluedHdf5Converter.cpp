#include "BluedHdf5Converter.h"
#include "AsyncDataQueue.h"
#include "BluedInputSource.h"
#include "H5Cpp.h"

using namespace H5;

void BluedHdf5Converter::convertToHdf5(const std::string &input_file, const std::string &output_file) {
    BluedInputSource input;
    input.readWholeLocation(input_file);

    try {
        this->createNewDataSpace(output_file);
        this->writeData(input.data_manager);
    } catch (...) {

    }
    input.stopGracefully();
}

void BluedHdf5Converter::writeData(BluedDataManager &mgr) {
    hsize_t offset[2] = {0, 0};


    const int buffer_size = 1000;
    BluedDataPoint buffer[buffer_size];

    int count = 0;
    while (true) {
        BluedDataPoint *buffer_end = mgr->popDataPoints(buffer, &buffer[buffer_size]);
        hsize_t buffer_len =static_cast<hsize_t>(buffer_end - buffer);
        if (buffer_len == 0) {
            break;
        }
        hsize_t size[2] = {offset[0] + buffer_len, this->dims[1]};
        dataset.extend(size);
        hsize_t hyper_slab_dimensions[2] = {buffer_len, this->dims[1]};
        DataSpace mspace(rank, hyper_slab_dimensions);


        DataSpace fspace = dataset.getSpace();

        fspace.selectHyperslab(H5S_SELECT_SET, hyper_slab_dimensions, offset);
        dataset.write(reinterpret_cast<const void*>(buffer), PredType::NATIVE_FLOAT, mspace, fspace);
        offset[0] += buffer_len;
        ++count;
        if(count % 1000 == 0) {
            std::cout << "Converted elements: " << offset[0]  << std::endl;
        }
    }
    std::cout << "Total converted elements: " << offset[0]  << std::endl;
}

void BluedHdf5Converter::createNewDataSpace(const std::string &output_file) {

    H5std_string FILE_NAME(output_file.c_str());
    H5std_string DATASET_NAME("data");

    file = H5File(FILE_NAME, H5F_ACC_TRUNC);


    DataType datatype(PredType::NATIVE_FLOAT);


    hsize_t max_dims[rank] = {H5S_UNLIMITED, ncols};

    this->file_data_space = DataSpace(rank, this->dims, max_dims);
    DSetCreatPropList cparms;
    hsize_t chunk_dims[rank] = {chunk_size, ncols};
    cparms.setChunk(rank, chunk_dims);

    this->dataset = file.createDataSet(DATASET_NAME, datatype, file_data_space, cparms);
}


