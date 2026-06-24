#include "MetricsEngine.h"
#include <algorithm>
//heyfh
#include <cmath>

namespace {
double toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}
}  // namespace

double MetricsEngine::groundTrackDistance(const TelemetryReading& a,
                                          const TelemetryReading& b) const {
    // Haversine formula: great-circle distance between two points on a sphere
    // given their latitude/longitude. Standard approach for surface distance.
    const double lat1 = toRadians(a.latitude);
    const double lat2 = toRadians(b.latitude);
    const double dlat = toRadians(b.latitude - a.latitude);
    const double dlon = toRadians(b.longitude - a.longitude);

    const double sin_dlat = std::sin(dlat / 2.0);
    const double sin_dlon = std::sin(dlon / 2.0);

    const double h = sin_dlat * sin_dlat +
                     std::cos(lat1) * std::cos(lat2) * sin_dlon * sin_dlon;
    const double c = 2.0 * std::asin(std::min(1.0, std::sqrt(h)));

    return kEarthRadiusKm * c;
}

Metrics MetricsEngine::compute(const TelemetryReading& previous,
                               const TelemetryReading& current) const {
    Metrics m;
    m.ground_track_km = groundTrackDistance(previous, current);
    m.velocity_delta_kmh = current.velocity_kmh - previous.velocity_kmh;
    m.seconds_elapsed =
        static_cast<double>(current.timestamp - previous.timestamp);
    return m;
}
