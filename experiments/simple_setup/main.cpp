#include "SmartApp.h"
#include <iostream>
#include <ProgramConfiguration.h>
#include "BluedHdf5InputSource.h"


int main(int argc, char **argv) {
    using namespace std;
    if (argc < 3) {
        cout << "usage: simple_setup <config_file> <blued_file> \n";
        return 0;
    }
    ProgramConfiguration config;
    if(!config.load(argv[1])) {
        cerr << "Could not load config\n";
        return -1;
    }
    PowerMetaData meta_data = config.toPowerMetaData();


    BluedHdf5InputSource input_source;
    std::shared_ptr<DynamicStreamMetaData> dynamic_meta_data(new DynamicStreamMetaData);
    dynamic_meta_data->setFixedPowerMetaData(meta_data);
    std::shared_ptr<AsyncDataQueue<BluedDataPoint>> queue(new AsyncDataQueue<BluedDataPoint>);
    input_source.startReading(queue, dynamic_meta_data, argv[2], config.data_set_start_time);

    SmartApp<BluedDataPoint> smart_app(dynamic_meta_data,queue);

    smart_app.getEventDetector()->event_detected.connect([](std::shared_ptr<Event<BluedDataPoint>> ev) {
        std::cout << ev->event_meta_data.event_time << std::endl;
    });
    smart_app.getEventDetector()->setThreshold(0.1);

    while(smart_app.getDataStreamDistributor()->loopWorkersOnce(queue)) {


    }


    return 0;
}
