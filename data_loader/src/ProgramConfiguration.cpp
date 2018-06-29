
#include "ProgramConfiguration.h"

#include <string.h>
#include <sstream>
#include <iostream>
#include <exception>


#include "ini.h"

template<typename T> T lexical_cast(const char *str) {
    T var;
    std::istringstream iss;
    iss.exceptions(std::istringstream::badbit | std::istringstream::failbit);
    iss.str(str);
    iss >> var;
    return var;
}

template<>
std::string lexical_cast<std::string> (const char *str) {
    return std::string(str);
}

/**
 * @brief This function converts the value to the type T and stores it in the destination if it is the value we want to store.
 *
 * @param expected_section The ini section we expect.
 * @param expected_name The name of the attribute we expect.
 * @param section The actual section.
 * @param name The actual name.
 * @param value The value of the attribute.
 * @param destination The destination where we want to store our attribute.
 * @return Returns true if this is the attribute we expect, false otherwise.
 *
 * @throws Exception An exception if the reading fails
 *
 */
template<typename T> bool
mapSetting(const char *expected_section, const char *expected_name, const char *section, const char *name,
           const char *value, T &destination) {
    if (strcmp(name, expected_name) != 0) {
        return false;
    }
    if (strcmp(section, expected_section) != 0) {
        return false;
    }
    destination = lexical_cast<T>(value);
    return true;
}

static int handler(void *user, const char *section, const char *name, const char *value) {
    try {
        ProgramConfiguration *calib = reinterpret_cast<ProgramConfiguration *> (user);

        bool success = mapSetting<unsigned long>("main", "sample_rate", section, name, value, calib->sample_rate);

        success |= mapSetting<unsigned long>("main", "frequency", section, name, value, calib->frequency);
        success |= mapSetting<float>("main", "voltage", section, name, value, calib->voltage);
        success |= mapSetting<std::string>("main", "data_set_start_time", section, name, value,
                                           calib->data_set_start_time);

        success |= mapSetting<unsigned long>("data", "max_data_points_in_queue", section, name, value,
                                             calib->max_data_points_in_queue);
        success |= mapSetting<int>("data", "data_points_stored_of_event", section, name, value,
                                   calib->data_points_stored_of_event);
        success |= mapSetting<int>("data", "data_points_stored_before_event", section, name, value,
                                   calib->data_points_stored_before_event);


        return success;
    } catch (std::exception &e) {
        return false;
    }
}

bool ProgramConfiguration::load(const std::string &file_path) {
    if (ini_parse(file_path.c_str(), handler, this) < 0) {
        std::cout << "Could not load config." << std::endl;
        return false;
    }
    return true;
}

PowerMetaData ProgramConfiguration::toPowerMetaData() {
    PowerMetaData result;
    result.data_points_stored_of_event = this->data_points_stored_of_event;
    result.sample_rate = this->sample_rate; /**< The sample rate of the current. 12000 for BLUED, 16000 for UK-DALE */
    result.frequency = this->frequency; /**< The frequency in Hz of the current. */
    result.voltage = this->voltage; /**< The voltage of the current. In the European  Union 230 V is the standard */

    result.max_data_points_in_queue = this->max_data_points_in_queue; /**< The number of samples we store unitl the writing thread is blocked.*/
    result.data_points_stored_of_event = this->data_points_stored_of_event; /**< The number of samples we store until the writing thread is blocked.*/
    result.data_points_stored_before_event = this->data_points_stored_before_event;
    return result;
}


std::ostream &operator<<(std::ostream &stream, const ProgramConfiguration &meta_data) {

    stream << "sample_rate: " << meta_data.sample_rate << "\n";
    stream << "frequency: " << meta_data.frequency << "\n";
    stream << "voltage: " << meta_data.voltage << "\n";
    stream << "data_set_start_time: " << meta_data.data_set_start_time << "\n";

    stream << "max_data_points_in_queue: " << meta_data.max_data_points_in_queue << "\n";
    stream << "data_points_stored_of_event: " << meta_data.data_points_stored_of_event << "\n";
    stream << "data_points_stored_before_event " << meta_data.data_points_stored_before_event << "\n";



    return stream;
}
