find_package(Boost REQUIRED COMPONENTS system filesystem unit_test_framework)

include_directories(
    ${Boost_INCLUDE_DIRS}
)

add_definitions(-DBOOST_TEST_DYN_LINK)
add_executable(Test
    src/test.cpp
    src/smart_app_test.cpp src/stream_distributer_test.cpp src/data_stream_generators.h src/event_detection_test.cpp  src/data_storage_test.cpp)

target_link_libraries(Test

    data_loader
    libanalyze
    event_detector
    data_analyzer
    smart_app
    ${Boost_LIBRARIES}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    -lssl -lcrypto
    )

add_test(BaseTest
    COMMAND Test)
