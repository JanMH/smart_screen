
#ifndef SMART_SCREEN_NOFARECINTERFACE_H
#define SMART_SCREEN_NOFARECINTERFACE_H


extern "C" {

bool setupCInterface(const char *configFile);

void addDataPoint(float voltage, float ampere);


}


#endif //SMART_SCREEN_NOFARECINTERFACE_H
