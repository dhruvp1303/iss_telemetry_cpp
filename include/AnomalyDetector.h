#ifndef ANOMALY_DETECTOR_H
#define ANOMALY_DETECTOR_H

#include <string>
#include <vector>

#include "MetricsEngine.h"
#include "TelemetryReading.h"

// Configurable thresholds defining the expected operating envelope.
// Defaults bracket the real ISS: it orbits at ~400-420 km and moves at
// ~27,500 km/h, so these bands flag readings that fall outside normal
// operation or a physically implausible jump between samples.
struct AnomalyThresholds {
    double min_altitude_km = 370.0;
    double max_altitude_km = 460.0;
    double max_velocity_kmh = 30000.0;
    double max_velocity_delta_kmh = 2000.0;  // max plausible speed change/step
};

// One flagged condition: which rule fired and a human-readable reason.
struct Anomaly {
    std::string rule;
    std::string reason;
};

// Checks readings (and the metrics derived from them) against the configured
// thresholds.
//
// Interface contract:
//   check(reading, metrics) -> list of Anomaly entries, one per violated rule.
//   An empty list means the reading is within the expected envelope.
class AnomalyDetector {
public:
    explicit AnomalyDetector(AnomalyThresholds thresholds = {});

    std::vector<Anomaly> check(const TelemetryReading& reading,
                               const Metrics& metrics) const;

private:
    AnomalyThresholds thresholds_;
};

#endif  // ANOMALY_DETECTOR_H
