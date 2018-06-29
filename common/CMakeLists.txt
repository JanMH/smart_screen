cmake_minimum_required(VERSION 2.8)
project(common)


set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(COMMON_SRCS
    ${PROJECT_SOURCE_DIR}/Logger.cpp
    )

set(SPDLOG_INCLUDE_DIR ${EXTERNAL_DEPENDENCIES_DIR}/spdlog/include)
find_package(Boost COMPONENTS REQUIRED)


add_library(${PROJECT_NAME} ${COMMON_SRCS})

target_include_directories(${PROJECT_NAME} SYSTEM
    PUBLIC
    "${PROJECT_SOURCE_DIR}"
    ${Boost_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIR}
    )


target_link_libraries(${PROJECT_NAME}
    ${BOOST_LIBRARIES}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
