#ifndef ANOMALY_DETECTOR_H
#define ANOMALY_DETECTOR_H

#include <string>
#include <vector>

#include "MetricsEngine.hpp"
#include "TelemetryReading.hpp"


struct AnomalyThresholds {
    double min_altitude_km = 370.0;
    double max_altitude_km = 460.0;
    double max_velocity_kmh = 30000.0;
    double max_velocity_delta_kmh = 2000.0;  // max plausible speed change/step
};

struct Anomaly {
    std::string rule;
    std::string reason;
};

class AnomalyDetector {
public:
    explicit AnomalyDetector(AnomalyThresholds thresholds = {});

    std::vector<Anomaly> check(const TelemetryReading& reading,
                               const Metrics& metrics) const;

private:
    AnomalyThresholds thresholds_;
};

#endif  // ANOMALY_DETECTOR_H
