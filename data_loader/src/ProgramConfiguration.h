
#ifndef SMART_SCREEN_PROGRAMCONFIGURATION_H
#define SMART_SCREEN_PROGRAMCONFIGURATION_H

#include <string>
#include "PowerMetaData.h"

/**
 * @brief Contains configuration parameters for the behavior of the classification and detection
 */
struct ProgramConfiguration {

    /**
     * @brief Loads a file configuration into the attributes of this class.
     *
     * @return Returns false on failure.
     */
    bool load(const std::string &file_path);

    // attributes
    unsigned long sample_rate = 12000; /**< The sample rate of the current. 12000 for BLUED, 16000 for UK-DALE */
    unsigned long frequency = 60; /**< The frequency in Hz of the current. */
    float voltage = 230; /**< The voltage of the current. In the European  Union 230 V is the standard */
    std::string data_set_start_time = "";

    unsigned long max_data_points_in_queue = 16000; /**< The number of samples we store unitl the writing thread is blocked.*/
    int data_points_stored_of_event = 0; /**< When detecting an event we store this number of data points before the event to reason about the state before the event */
    int data_points_stored_before_event = 0;

    PowerMetaData toPowerMetaData();


};
std::ostream &operator<<(std::ostream &stream, const ProgramConfiguration &meta_data);

#endif //SMART_SCREEN_PROGRAMCONFIGURATION_H
