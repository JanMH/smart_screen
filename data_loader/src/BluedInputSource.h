#ifndef _BLUED_INPUT_SOURCE_
#define _BLUED_INPUT_SOURCE_

#include <string>
#include <thread>
#include <vector>

#include "BluedDefinitions.h"

/**
 * @brief Reads data from the original blued text files. This process is very slow. If possible use the BluedHdf5Input source.
 */
class BluedInputSource {


public:
    void startReading(const std::string &file_path);

    void startReading(const std::string &file_path, std::function<void()> callback);

    void readWholeLocation(const std::string &directory, std::function<void()> callback);

    void readWholeLocation(const std::string &directory);

    void stopGracefully();

    void stopNow();

    ~BluedInputSource() {
        this->stopNow();
        this->stopGracefully();
    }

public:
    BluedDataManager data_manager;


private:
    void run(const std::string &file_path, std::function<void()> callback);

    void readFile(const std::string &file_path);

    void runLocations(std::vector<std::string> locations, std::function<void()> callback);

    BluedDataPoint matchLine(std::ifstream &input_stream);

    bool readOnce(std::ifstream &input_stream);

    static bool skipToData(std::ifstream &input_stream);

private:
    bool continue_reading = true;
    std::thread runner;
    double previous_x_value = -1000;
};

#endif // _BLUED_INPUT_SOURCE_
