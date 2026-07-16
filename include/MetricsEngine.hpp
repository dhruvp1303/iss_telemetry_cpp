#ifndef METRICS_ENGINE_H
#define METRICS_ENGINE_H

#include "TelemetryReading.hpp"

struct Metrics {
    double ground_track_km = 0.0;   // surface distance travelled between samples
    double velocity_delta_kmh = 0.0;  // change in ground speed between samples
    double seconds_elapsed = 0.0;   // wall-clock gap between the two samples
};


class MetricsEngine {
public:
    double groundTrackDistance(const TelemetryReading& a,
                               const TelemetryReading& b) const;

    Metrics compute(const TelemetryReading& previous,
                    const TelemetryReading& current) const;

    static constexpr double kEarthRadiusKm = 6371.0;
};

#endif  // METRICS_ENGINE_H
