cmake_minimum_required(VERSION 2.8)
project(nofare_app)

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include(${CMAKE_SOURCE_DIR}/cmake/Scripts/AddDoxygenTarget.cmake)


# include cpp-netlib
include(${EXTERNAL_DEPENDENCIES_DIR}/cpp-netlib/build/cppnetlibConfig.cmake)

find_package(Boost REQUIRED)
set(COMMON_SRCS
    src/common/nofare_definitions.h
    src/common/AddLabelCommand.cpp
    src/common/AddLabelCommand.h
    src/common/ShutdownCommand.cpp
    src/common/ShutdownCommand.h
    src/common/ReclassifyCommand.cpp
    src/common/ReclassifyCommand.h
    )

add_library(${PROJECT_NAME}_server_lib
    src/server/EventHandlers.cpp
    src/server/EventHandlers.h
    src/server/CLIConnection.cpp
    src/server/CLIConnection.h
    src/server/InputSource.cpp
    src/server/InputSource.h
    src/server/ModelSerialization.cpp
    src/server/ModelSerialization.hpp
    src/server/Init.cpp
    src/server/Init.h
    src/server/NoFaReAppConfig.h
    ${COMMON_SRCS}
    )

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/server/main.cpp )

target_include_directories(${PROJECT_NAME}_server_lib
    PUBLIC
    ${PROJECT_SOURCE_DIR}/server
    ${PROJECT_SOURCE_DIR}/common)

target_link_libraries(${PROJECT_NAME}_server_lib

    common
    data_loader
    event_detector
    pthread
    smart_app
    ${BOOST_LIBRARIES}
    )
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_server_lib)

add_executable(${PROJECT_NAME}_client
    ${PROJECT_SOURCE_DIR}/client/main.cpp ${COMMON_SRCS})
target_include_directories(${PROJECT_NAME}_client
    PRIVATE
    ${PROJECT_SOURCE_DIR}/client
    ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(${PROJECT_NAME}_client
    pthread
    ${BOOST_LIBRARIES})

if (UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} rt)
    target_link_libraries(${PROJECT_NAME}_client rt)
endif ()


install(TARGETS ${PROJECT_NAME} DESTINATION bin)
