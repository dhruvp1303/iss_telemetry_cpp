#ifndef TELEMETRY_READING_H
#define TELEMETRY_READING_H

#include <cstdint>


struct TelemetryReading {
    std::int64_t timestamp = 0;   // Unix epoch seconds
    double latitude = 0.0;        // degrees, -90..90
    double longitude = 0.0;       // degrees, -180..180
    double altitude_km = 0.0;     // kilometers above sea level
    double velocity_kmh = 0.0;    // ground speed, km/h
};

#endif  // TELEMETRY_READING_H
