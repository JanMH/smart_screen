cmake_minimum_required(VERSION 2.8)
project(data_analyzer)

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include(${CMAKE_SOURCE_DIR}/cmake/Scripts/AddDoxygenTarget.cmake)


# include libnabo
include(${EXTERNAL_DEPENDENCIES_DIR}/libnabo/build/libnaboConfig.cmake)

find_package(Boost COMPONENTS system filesystem date_time serialization REQUIRED)



add_library(${PROJECT_NAME}
    src/DataClassifier.h
    src/EventLabelManager.h src/ClassificationConfig.cpp src/ClassificationConfig.h src/FeatureExtractor.h)


target_link_libraries(${PROJECT_NAME}
    data_loader
    event_detector
    common
    ${libnabo_LIBRARIES}
    ${Boost_Libraries})
target_include_directories(${PROJECT_NAME} SYSTEM
    PUBLIC
    ${Boost_INCLUDE_DIRS}
    )

target_include_directories(${PROJECT_NAME} SYSTEM
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${libnabo_INCLUDE_DIRS})
message("nabo_libs: ${libnabo_INCLUDE_DIRS}")


install(TARGETS ${PROJECT_NAME} DESTINATION bin)
