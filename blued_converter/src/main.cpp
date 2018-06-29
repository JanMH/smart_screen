#include <iostream>
#include <thread>
#include "BluedHdf5Converter.h"

#include "PowerMetaData.h"
#include "AsyncDataQueue.h"
#include "BluedHdf5InputSource.h"


int main(int argc, char **argv) {
    if(argc < 3) {
        std::cout << "usage: blued_converter <directory to files> <outfile>\n";
        return -1;
    }
    BluedHdf5Converter converter;
    converter.convertToHdf5(std::string(argv[1]), std::string(argv[2]));
    return 0;
}
