#ifndef _DATA_POINT_H_
#define _DATA_POINT_H_


/**
 * @brief This structure holds one datum for voltage and ampere power
 * 
 */
struct DefaultDataPoint {
    using datum = float;

     DefaultDataPoint(float volt_value, float ampere_value) : volts(volt_value), amps(ampere_value) {}

     DefaultDataPoint() : volts(0), amps(0) {}


     datum voltage() const { return volts; }

     void voltage(datum v) { this->volts = v; }

     datum ampere() const { return amps; }

     void ampere(datum a) { this->amps = a; }

    bool operator==(const DefaultDataPoint& other) const {
        return this->voltage() == other.voltage() && this->ampere() == other.ampere();
    }

    datum volts;
    datum amps;
};

#endif // _DATA_POINT_H_
