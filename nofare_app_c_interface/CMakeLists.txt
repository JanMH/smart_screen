cmake_minimum_required(VERSION 2.8)
project(nofare_app_c_interface)


set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(nofare_app_c_interface_INCLUDE_DIR ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})



set(NOFARE_C_SRCS
    src/NofareCInterface.h
    src/NofareCInterface.cpp)


find_package(Boost COMPONENTS system filesystem date_time serialization REQUIRED)
find_package(HDF5 COMPONENTS CXX REQUIRED)


add_library(${PROJECT_NAME} ${NOFARE_C_SRCS})

target_include_directories(${PROJECT_NAME} SYSTEM
    PUBLIC
    "${nofare_app_c_interface_INCLUDE_DIR}"
    ${Boost_INCLUDE_DIRS}
    ${HDF5_INCLUDE_DIR} ${HDF5_INCLUDE_DIRS}
    )


target_link_libraries(${PROJECT_NAME} nofare_app_server_lib)


message("Boost Libraries linked to ${PROJECT_NAME}: ${Boost_LIBRARIES}")

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
