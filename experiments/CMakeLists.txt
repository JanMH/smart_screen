cmake_minimum_required(VERSION 2.8)
project(experiments)

find_package(Boost COMPONENTS program_options REQUIRED)

set(experiment_deps data_loader libanalyze event_detector data_analyzer smart_app ${Boost_LIBRARIES})
include_directories(${Boost_INCLUDE_DIRS})

add_executable(simple_setup
    simple_setup/main.cpp
    )

add_executable(event_classification_setup
    event_classification_setup/main.cpp
    event_classification_setup/CrossValidationResult.h
    ../data_analyzer/src/SerializeEventLabelManager.h
    event_classification_setup/SelectPartitions.h)

add_executable(data_vis
    data_vis/main.cpp
    event_classification_setup/CrossValidationResult.h
    ../data_analyzer/src/SerializeEventLabelManager.h
    data_vis/EventVisualizer.h
    data_vis/ClassificationVisualizer.h
    data_vis/CrossValidationStats.h)

add_executable(slimmed_validation
    event_classification_setup/slimmed_validation.cpp
    event_classification_setup/CrossValidationResult.h
    ../data_analyzer/src/SerializeEventLabelManager.h
    )


target_link_libraries(simple_setup ${experiment_deps})

target_include_directories(simple_setup PRIVATE ${dirs})

target_link_libraries(event_classification_setup ${experiment_deps})
target_link_libraries(data_vis ${experiment_deps})
target_include_directories(data_vis PRIVATE event_classification_setup)
target_link_libraries(slimmed_validation ${experiment_deps})

