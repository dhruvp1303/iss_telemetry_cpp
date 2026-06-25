#include <gtest/gtest.h>

#include <cmath>

#include "AnomalyDetector.h"
#include "MetricsEngine.h"
#include "TelemetryParser.h"
#include "TelemetryReading.h"


TEST(TelemetryParser, ParsesWellFormedLine) {
    TelemetryParser parser;
    const std::string line =
        R"({"timestamp":1700000000,"latitude":40.7128,"longitude":-74.0060,)"
        R"("altitude":408.5,"velocity":27600.0})";

    const auto reading = parser.parse(line);
    ASSERT_TRUE(reading.has_value());
    EXPECT_EQ(reading->timestamp, 1700000000);
    EXPECT_NEAR(reading->latitude, 40.7128, 1e-6);
    EXPECT_NEAR(reading->longitude, -74.0060, 1e-6);
    EXPECT_NEAR(reading->altitude_km, 408.5, 1e-6);
    EXPECT_NEAR(reading->velocity_kmh, 27600.0, 1e-6);
}

TEST(TelemetryParser, HandlesValuesEncodedAsStrings) {
    TelemetryParser parser;
    const std::string line =
        R"({"timestamp":"1700000000","latitude":"-12.5","longitude":"100.25",)"
        R"("altitude":"410","velocity":"27500"})";

    const auto reading = parser.parse(line);
    ASSERT_TRUE(reading.has_value());
    EXPECT_NEAR(reading->latitude, -12.5, 1e-6);
    EXPECT_NEAR(reading->longitude, 100.25, 1e-6);
}

TEST(TelemetryParser, RejectsMissingField) {
    TelemetryParser parser;
    const std::string line =
        R"({"timestamp":1700000000,"latitude":40.0,"longitude":-74.0,)"
        R"("altitude":408.0})";

    EXPECT_FALSE(parser.parse(line).has_value());
}

TEST(TelemetryParser, RejectsGarbageInput) {
    TelemetryParser parser;
    EXPECT_FALSE(parser.parse("not json at all").has_value());
    EXPECT_FALSE(parser.parse("").has_value());
}


TEST(MetricsEngine, GroundTrackZeroForSamePoint) {
    MetricsEngine engine;
    TelemetryReading a;
    a.latitude = 51.5;
    a.longitude = -0.12;
    EXPECT_NEAR(engine.groundTrackDistance(a, a), 0.0, 1e-9);
}

TEST(MetricsEngine, GroundTrackKnownDistance) {
    MetricsEngine engine;
    TelemetryReading london;
    london.latitude = 51.5074;
    london.longitude = -0.1278;
    TelemetryReading paris;
    paris.latitude = 48.8566;
    paris.longitude = 2.3522;

    const double d = engine.groundTrackDistance(london, paris);
    EXPECT_NEAR(d, 343.0, 5.0);  
}

TEST(MetricsEngine, GroundTrackIsSymmetric) {
    MetricsEngine engine;
    TelemetryReading a;
    a.latitude = 10.0;
    a.longitude = 20.0;
    TelemetryReading b;
    b.latitude = -5.0;
    b.longitude = 80.0;
    EXPECT_NEAR(engine.groundTrackDistance(a, b),
                engine.groundTrackDistance(b, a), 1e-9);
}

TEST(MetricsEngine, ComputesVelocityDeltaAndElapsed) {
    MetricsEngine engine;
    TelemetryReading prev;
    prev.timestamp = 1000;
    prev.velocity_kmh = 27500.0;
    TelemetryReading curr;
    curr.timestamp = 1005;
    curr.velocity_kmh = 27650.0;

    const Metrics m = engine.compute(prev, curr);
    EXPECT_NEAR(m.velocity_delta_kmh, 150.0, 1e-9);
    EXPECT_NEAR(m.seconds_elapsed, 5.0, 1e-9);
}


TEST(AnomalyDetector, NominalReadingHasNoAnomalies) {
    AnomalyDetector detector;
    TelemetryReading r;
    r.altitude_km = 408.0;
    r.velocity_kmh = 27600.0;
    Metrics m;
    m.velocity_delta_kmh = 50.0;

    EXPECT_TRUE(detector.check(r, m).empty());
}

TEST(AnomalyDetector, FlagsAltitudeAboveCeiling) {
    AnomalyDetector detector;
    TelemetryReading r;
    r.altitude_km = 500.0;  // above default max of 460
    r.velocity_kmh = 27600.0;
    Metrics m;

    const auto anomalies = detector.check(r, m);
    ASSERT_EQ(anomalies.size(), 1u);
    EXPECT_EQ(anomalies[0].rule, "altitude_high");
}

TEST(AnomalyDetector, FlagsVelocityJump) {
    AnomalyDetector detector;
    TelemetryReading r;
    r.altitude_km = 408.0;
    r.velocity_kmh = 27600.0;
    Metrics m;
    m.velocity_delta_kmh = 5000.0;  // above default max delta of 2000

    const auto anomalies = detector.check(r, m);
    ASSERT_EQ(anomalies.size(), 1u);
    EXPECT_EQ(anomalies[0].rule, "velocity_jump");
}

TEST(AnomalyDetector, ReportsMultipleViolationsAtOnce) {
    AnomalyDetector detector;
    TelemetryReading r;
    r.altitude_km = 300.0;       // below min
    r.velocity_kmh = 35000.0;    // above max
    Metrics m;
    m.velocity_delta_kmh = 50.0;

    const auto anomalies = detector.check(r, m);
    EXPECT_EQ(anomalies.size(), 2u);
}

TEST(AnomalyDetector, RespectsCustomThresholds) {
    AnomalyThresholds tight;
    tight.max_altitude_km = 410.0;
    AnomalyDetector detector(tight);

    TelemetryReading r;
    r.altitude_km = 420.0;  // fine under defaults, too high under custom
    r.velocity_kmh = 27600.0;
    Metrics m;

    const auto anomalies = detector.check(r, m);
    ASSERT_EQ(anomalies.size(), 1u);
    EXPECT_EQ(anomalies[0].rule, "altitude_high");
}
