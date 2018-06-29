cmake_minimum_required(VERSION 2.8)
project(event_detector)

set(event_detector_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR})

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(${CMAKE_SOURCE_DIR}/cmake/Scripts/AddDoxygenTarget.cmake)
find_package(Boost COMPONENTS system filesystem date_time REQUIRED)




add_library(${PROJECT_NAME} STATIC
    src/EventMetaData.h
    src/EventStorage.h
    src/EventDetector.h
    src/dummy.cpp
    src/Event.h
    ../data_analyzer/src/EventFeatures.h src/DataStreamWorker.h src/DataStreamDistributor.h src/DataStreamTypes.h src/Hdf5EventStorage.h src/Hdf5Serializations.h)

target_link_libraries(${PROJECT_NAME} libanalyze)
target_link_libraries(${PROJECT_NAME} data_loader)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${event_detector_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    )


install(TARGETS ${PROJECT_NAME} DESTINATION bin)
