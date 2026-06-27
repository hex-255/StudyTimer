#ifndef FUELGAUGE_H
#define FUELGAUGE_H

#include "defines.h"


// Initialize the fuel gauge
inline bool initFuelGauge() {
    return lipo.begin();
}

// Set the battery capacity in mAh
inline void setFuelGaugeCapacity(uint16_t capacity) {
    lipo.setCapacity(capacity);
}

inline void setTerminateVoltage(uint16_t voltage) {
    lipo.setTerminateVoltage(voltage);
}

// Get battery voltage in mV
inline uint16_t getFuelGaugeVoltage() {
    return lipo.voltage();
}

// Get battery current in mA (average)
inline int16_t getFuelGaugeCurrent() {
    return lipo.current(AVG);
}

// Get state of charge in %
inline uint16_t getFuelGaugeSOC() {
    return lipo.soc();
}

// Get full charge capacity in mAh
inline uint16_t getFuelGaugeCapacity() {
    return lipo.capacity(FULL);
}

// Get remaining capacity in mAh
inline uint16_t getFuelGaugeRemainingCapacity() {
    return lipo.capacity(REMAIN);
}

// Get temperature in Celsius
inline uint16_t getFuelGaugeTemperature() {
    return lipo.temperature();
}

#endif // FUELGAUGE_H