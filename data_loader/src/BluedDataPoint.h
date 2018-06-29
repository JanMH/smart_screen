#ifndef _BLUED_DATA_POINT_H_
#define _BLUED_DATA_POINT_H_


/**
 * @brief This structure holds one datum for voltage and two for ampere.
 * 
 */
struct BluedDataPoint {
    typedef float datum;

    BluedDataPoint(datum _x_value, datum _current_a, datum _current_b, datum _voltage_a) : x_value(_x_value),
                                                                                           current_a(_current_a),
                                                                                           current_b(_current_b),
                                                                                           voltage_a(_voltage_a) {}

    BluedDataPoint() : x_value(0), current_a(0), current_b(0), voltage_a(0) {}

    BluedDataPoint(const BluedDataPoint &other) : x_value(other.x_value), current_a(other.current_a),
                                                  current_b(other.current_b), voltage_a(other.voltage_a) {}


    datum voltage() const { return -voltage_a; }

    void voltage(datum v) { this->voltage_a = v; }

    datum ampere() const { return current_b; }

    void ampere(datum a) { this->current_b = a; }

    float x_value;

    float current_a;
    float current_b;
    float voltage_a;

};

#endif // _DATA_POINT_H_
