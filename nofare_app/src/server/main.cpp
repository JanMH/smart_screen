


#include "Init.h"

int main(int argc, char *argv[]) {
    // Console logger with color
    program_log->set_level(spdlog::level::info);

#ifdef USE_BLUED

    if (argc < 3) {
        std::cerr << "usage: nofare_app <config_file> <blued file> \n";
        return 0;
    }
    run<BluedDataPoint>(argv[1], argv[2]);
#else
    if (argc < 2) {
        std::cerr << "usage: nofare_app <config_file>\n";
        return 0;
    }
    run<DefaultDataPoint>(argv[1]);

#endif


}
