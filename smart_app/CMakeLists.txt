cmake_minimum_required(VERSION 2.8)
project(smart_app)


set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)


include(${CMAKE_SOURCE_DIR}/cmake/Scripts/AddDoxygenTarget.cmake)




set(PROJECT_SOURCES src/SmartApp.h src/MetricProvider.h src/dummy_funtion.cpp src/dummy_funtion.h)


find_package(Boost COMPONENTS system filesystem date_time serialization REQUIRED)
find_package(HDF5 COMPONENTS CXX REQUIRED)


add_library(${PROJECT_NAME} ${PROJECT_SOURCES} )

target_include_directories(${PROJECT_NAME} SYSTEM
    PUBLIC
    "${PROJECT_SOURCE_DIR}"
    ${Boost_INCLUDE_DIRS}
    ${HDF5_INCLUDE_DIR} ${HDF5_INCLUDE_DIRS}
    )


target_link_libraries(${PROJECT_NAME} -lm -lpthread)
target_link_libraries(${PROJECT_NAME}
    ${Boost_LIBRARIES}
    ${HDF5_LIBRARIES}
    data_loader
    data_analyzer
    event_detector
    )

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
