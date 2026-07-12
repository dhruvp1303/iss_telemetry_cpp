#ifndef METRICS_ENGINE_H
#define METRICS_ENGINE_H

#include "TelemetryReading.h"

// Derived metrics computed from a pair of consecutive readings.
struct Metrics {
    double ground_track_km = 0.0;   // surface distance travelled between samples
    double velocity_delta_kmh = 0.0;  // change in ground speed between samples
    double seconds_elapsed = 0.0;   // wall-clock gap between the two samples
};

// Computes derived metrics from telemetry readings.
//
// Interface contract:
//   groundTrackDistance(a, b) -> great-circle surface distance in km between the
//                                two readings' lat/lon, via the haversine formula.
//   compute(previous, current) -> full Metrics for the step from previous to
//                                 current.
//
// All methods are pure functions of their inputs (no internal state), which
// keeps them trivially testable.
class MetricsEngine {
public:
    double groundTrackDistance(const TelemetryReading& a,
                               const TelemetryReading& b) const;

    Metrics compute(const TelemetryReading& previous,
                    const TelemetryReading& current) const;

    static constexpr double kEarthRadiusKm = 6371.0;
};

#endif  // METRICS_ENGINE_H
