/** @file */
#include <string>
#include <ostream>

#include <iostream>

#ifndef _CONFIG_LOADER_H_
#define _CONFIG_LOADER_H_


/**
  * @brief The FileConfig holds all relevant meta data about our electrical data including sample rate, conversion factors and so on.
  */
struct PowerMetaData {

    // attributes
    unsigned long sample_rate = 12000; /**< The sample rate of the current. 12000 for BLUED, 16000 for UK-DALE */
    unsigned long frequency = 60; /**< The frequency in Hz of the current. */
    float voltage = 230; /**< The voltage of the current. In the European  Union 230 V is the standard */

    unsigned long max_data_points_in_queue = 16000; /**< The number of samples we store unitl the writing thread is blocked.*/
    int data_points_stored_of_event = 0; /**< The number of samples we store until the writing thread is blocked.*/
    int data_points_stored_before_event = 0;

    unsigned long dataPointsPerPeriod() const {
        return sample_rate / frequency;
    }


};


#endif
