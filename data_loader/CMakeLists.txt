cmake_minimum_required(VERSION 2.8)
project(data_loader)


set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(dataloader_INCLUDE_DIR ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})


include(${CMAKE_SOURCE_DIR}/cmake/Scripts/AddDoxygenTarget.cmake)

# add ini file parser 
set(INIH_SOURCES
    "${EXTERNAL_DEPENDENCIES_DIR}/inih/ini.c"
    )
include_directories("${EXTERNAL_DEPENDENCIES_DIR}/inih")



set(DATALOADER_SRCS
    src/BluedDataPoint.h
    src/BluedDefinitions.h
    src/BluedHdf5InputSource.cpp
    src/BluedHdf5InputSource.h
    src/BluedInputSource.cpp
    src/BluedInputSource.h
    src/AsyncDataQueue.h
    src/DefaultDataPoint.h
    src/PowerMetaData.cpp
    src/PowerMetaData.h
    src/DynamicStreamMetaData.h
    src/DynamicStreamMetaData.cpp src/ProgramConfiguration.cpp src/ProgramConfiguration.h src/Hdf5Mutex.cpp src/Hdf5Mutex.h)


find_package(Boost COMPONENTS system filesystem date_time serialization REQUIRED)
find_package(HDF5 COMPONENTS CXX REQUIRED)


add_library(${PROJECT_NAME} ${DATALOADER_SRCS} ${INIH_SOURCES})

target_include_directories(${PROJECT_NAME} SYSTEM
    PUBLIC
    "${dataloader_INCLUDE_DIR}"
    ${Boost_INCLUDE_DIRS}
    ${HDF5_INCLUDE_DIR} ${HDF5_INCLUDE_DIRS}
    )


target_link_libraries(${PROJECT_NAME} -lm -lpthread)
target_link_libraries(${PROJECT_NAME}
    ${Boost_LIBRARIES}
    ${HDF5_LIBRARIES}
    common
    )
message("Boost Libraries linked to ${PROJECT_NAME}: ${Boost_LIBRARIES}")

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
