#include <boost/test/unit_test.hpp>
#include <SmartApp.h>
#include <BluedDataPoint.h>
#include <BluedHdf5InputSource.h>
#include "DataStreamDistributor.h"


BOOST_AUTO_TEST_CASE(smart_app_test) {
    SmartApp<BluedDataPoint> smart_app;
    BluedHdf5InputSource source;
    source.data_manager = smart_app.getDataQueue();
}

