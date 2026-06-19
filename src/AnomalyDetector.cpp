#include "AnomalyDetector.h"

#include <cmath>

AnomalyDetector::AnomalyDetector(AnomalyThresholds thresholds)
    : thresholds_(thresholds) {}

std::vector<Anomaly> AnomalyDetector::check(const TelemetryReading& reading,
                                            const Metrics& metrics) const {
    std::vector<Anomaly> anomalies;

    if (reading.altitude_km < thresholds_.min_altitude_km) {
        anomalies.push_back(
            {"altitude_low", "altitude below expected minimum"});
    }
    if (reading.altitude_km > thresholds_.max_altitude_km) {
        anomalies.push_back(
            {"altitude_high", "altitude above expected maximum"});
    }
    if (reading.velocity_kmh > thresholds_.max_velocity_kmh) {
        anomalies.push_back(
            {"velocity_high", "ground speed above expected maximum"});
    }
    if (std::fabs(metrics.velocity_delta_kmh) >
        thresholds_.max_velocity_delta_kmh) {
        anomalies.push_back(
            {"velocity_jump", "velocity changed more than expected between samples"});
    }

    return anomalies;
}
