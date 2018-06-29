
#include "NofareCInterface.h"

#include "Init.h"


#define C_INTERFACE_BUFFER_SIZE 1000

ConfiguredDataPoint buffer[C_INTERFACE_BUFFER_SIZE];
int buffer_pos = 0;

bool setupCInterface(const char *configFile) {
    run<ConfiguredDataPoint>(configFile, nullptr);
    return false;
}

void addDataPoint(float voltage, float ampere) {
    auto data_p = ConfiguredDataPoint();
    data_p.voltage(voltage);
    data_p.ampere(ampere);
    buffer[buffer_pos] = data_p;
    buffer_pos++;
    if(buffer_pos>= C_INTERFACE_BUFFER_SIZE) {
        smart_app.getDataQueue()->addDataPoints(buffer, buffer + C_INTERFACE_BUFFER_SIZE);
        buffer_pos = 0;
    }
}
