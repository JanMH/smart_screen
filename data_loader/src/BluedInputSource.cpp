#include "BluedInputSource.h"

#include <fstream>
#include <regex>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <iomanip>

void BluedInputSource::startReading(const std::string &file_path) {
    this->startReading(file_path, []() {});
}


void BluedInputSource::startReading(const std::string &file_path, std::function<void()> callback) {
    this->continue_reading = true;
    auto runner_function = std::bind(&BluedInputSource::run, this, file_path, callback);
    if(this->data_manager.get() == nullptr) {
        this->data_manager = std::shared_ptr<AsyncDataQueue<BluedDataPoint>>(new AsyncDataQueue<BluedDataPoint>());
    }
    this->runner = std::thread(runner_function);
}

void BluedInputSource::readWholeLocation(const std::string &directory) {
    this->readWholeLocation(directory, []() {});
}

void BluedInputSource::stopGracefully() {
    if (this->runner.joinable()) {
        this->continue_reading = false;
        this->runner.join();
    }
}

void BluedInputSource::stopNow() {
    this->continue_reading = false;
    this->data_manager->discardRestOfStream();

}

void BluedInputSource::readWholeLocation(const std::string &directory, std::function<void()> callback) {
    using namespace boost::filesystem;
    path p(directory);
    std::vector<std::string> locations;
    try {
        if (!is_directory(p)) {
            std::cerr << "Please Provide a directory";
        }
        recursive_directory_iterator begin(p), end;
        while (begin != end) {
            bool is_data_file = boost::algorithm::ends_with(begin->path().filename().c_str(), "txt");
            if (is_data_file) {
                locations.push_back(begin->path().string());
            }
            ++begin;
        }

    } catch (const filesystem_error &ex) {
        std::cerr << ex.what() << '\n';
    }


    std::sort(locations.begin(), locations.end());
    std::cout << "Considering the following locations for reading: \n";
    for(auto& location: locations) {
        std::cout <<location << std::endl;
    }

    this->continue_reading = true;
    auto runner_function = std::bind(&BluedInputSource::runLocations, this, locations, callback);
    this->runner = std::thread(runner_function);
}

void BluedInputSource::runLocations(std::vector<std::string> locations, std::function<void()> callback) {
    this->data_manager->restartStreaming();
    for (auto &file_path : locations) {
        if (!this->continue_reading) {
            break;
        }
        readFile(file_path);
    }
    this->data_manager->notifyStreamEnd();
    callback();
}


void BluedInputSource::run(const std::string &file_path, std::function<void()> callback) {
    this->data_manager->restartStreaming();
    readFile(file_path);
    this->data_manager->notifyStreamEnd();
    callback();
}

BluedDataPoint BluedInputSource::matchLine(std::ifstream &input_stream) {
    std::string line;

    double x_value;
    input_stream >> x_value;
    input_stream.ignore(50, ',');
    const double x_value_max_dist = 0.0001;
    if(x_value < previous_x_value || x_value > previous_x_value + x_value_max_dist) {
        if(previous_x_value > -100.f) {
            std::cerr << std::setprecision(9) << "The distance between the x_values is not within the bounds\nprevious value: "
                      << previous_x_value << "\ncurrent value: " << x_value
                      << "\ncurrent - prev: " <<x_value -  previous_x_value<< std::endl;
            throw std::exception();
        }
    }
    previous_x_value = x_value;

    float current_a;
    input_stream >> current_a;
    input_stream.ignore(50, ',');

    //ignore current b
    float current_b;
    input_stream >> current_b;

    input_stream.ignore(50, ',');

    float voltage_a;
    input_stream >> voltage_a;

    // skip to next line
    input_stream.ignore(50, '\n');

    return BluedDataPoint(static_cast<float>(x_value), current_a, current_b, voltage_a);
}

bool BluedInputSource::readOnce(std::ifstream &input_stream) {
    const unsigned int buffer_size = 10000;

    BluedDataPoint buffer[buffer_size];
    bool success = true;
    unsigned int i = 0;
    for (; i < buffer_size; ++i) {
        if (!(input_stream.good() && this->continue_reading)) {
            --i;
            success = false;
            break;

        }
        buffer[i] = BluedInputSource::matchLine(input_stream);

    }
    this->data_manager->addDataPoints(buffer, buffer + i);
    return success;
}


bool BluedInputSource::skipToData(std::ifstream &input_stream) {
    std::string line;
    while (input_stream.good()) {
        std::getline(input_stream, line);
        if (line == "X_Value,Current A,Current B,VoltageA,Comment\r") {
            return true;
        }
    }
    return false;
}

void BluedInputSource::readFile(const std::string &file_path) {
    std::ifstream input_stream;
    input_stream.open(file_path, std::ifstream::in);

    if (!BluedInputSource::skipToData(input_stream)) {
        std::cerr << "The file could not be opened: " << file_path << std::endl;

        throw std::exception();
    }
    while (readOnce(input_stream)) {
        // do nothing
    }
}

