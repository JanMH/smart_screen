cmake_minimum_required(VERSION 2.8)
project(blued_converter)


add_executable(${PROJECT_NAME}
        src/BluedHdf5Converter.cpp
        src/BluedHdf5Converter.h
        src/main.cpp
        )

target_link_libraries(${PROJECT_NAME} data_loader libanalyze event_detector data_analyzer)

target_include_directories(${PROJECT_NAME} SYSTEM
    PUBLIC
    )


install(TARGETS ${PROJECT_NAME} DESTINATION bin)
