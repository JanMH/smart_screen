#include <iostream>
#include <thread>
#include "EventStorage.h"
#include "PowerMetaData.h"
#include "AsyncDataQueue.h"
#include "BluedHdf5InputSource.h"

#include "DataClassifier.h"

#include "CrossValidationResult.h"
#include "SerializeEventLabelManager.h"
#include "SelectPartitions.h"


int main(int argc, char **argv) {

    using namespace std;


    if (argc < 3) {
        cout << "usage: event_detection_setup <event file> <result file> [<events direcotry>] \n";
        return 0;
    }
    EventStorage<BluedDataPoint> storage;
    if(argc >=4 ) {
        storage.event_directory = argv[3];
    }
    auto i = 0;

    DataClassifier<BluedDataPoint> analyzer;
    analyzer.startClassification(argv[1]);
    while (true) {
        try {
            auto event = storage.loadEvent(i);

            cout << "." << flush;
            analyzer.pushEvent(event);
            ++i;
        }
        catch (...) {
            break;
        }

    }
    cout << endl;


    analyzer.stopAnalyzingWhenDone();
        ofstream out_stream(argv[2]);
        boost::archive::text_oarchive b_archive(out_stream);
        auto label_manager  = analyzer.getEventLabelManager();
        b_archive << label_manager;




    return 0;
}
