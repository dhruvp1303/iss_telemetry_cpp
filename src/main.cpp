#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>

#include "AnomalyDetector.h"
#include "MetricsEngine.h"
#include "TelemetryParser.h"
#include "TelemetryReading.h"

namespace {

// Processes a stream of telemetry lines: parse -> compute metrics against the
// previous reading -> detect anomalies -> report. One pass per line keeps the
// per-message work bounded.
void processStream(std::istream& in) {
    TelemetryParser parser;
    MetricsEngine metrics_engine;
    AnomalyDetector detector;

    std::optional<TelemetryReading> previous;
    std::string line;
    std::size_t line_number = 0;
    std::size_t parsed = 0;
    std::size_t anomalies_found = 0;

    while (std::getline(in, line)) {
        ++line_number;
        if (line.empty()) {
            continue;
        }

        const auto reading = parser.parse(line);
        if (!reading) {
            std::cerr << "[warn] line " << line_number
                      << ": could not parse, skipping\n";
            continue;
        }
        ++parsed;

        if (!previous) {
            // First valid reading: nothing to diff against yet.
            std::cout << "[" << reading->timestamp << "] "
                      << "lat=" << std::fixed << std::setprecision(4)
                      << reading->latitude << " lon=" << reading->longitude
                      << " alt=" << std::setprecision(1) << reading->altitude_km
                      << "km  (baseline)\n";
            previous = reading;
            continue;
        }

        const Metrics m = metrics_engine.compute(*previous, *reading);
        const auto anomalies = detector.check(*reading, m);

        std::cout << "[" << reading->timestamp << "] "
                  << "track=" << std::fixed << std::setprecision(1)
                  << m.ground_track_km << "km  "
                  << "dV=" << std::showpos << m.velocity_delta_kmh << std::noshowpos
                  << "km/h";

        if (anomalies.empty()) {
            std::cout << "  OK\n";
        } else {
            std::cout << "  ANOMALY:";
            for (const auto& a : anomalies) {
                std::cout << " [" << a.rule << ": " << a.reason << "]";
                ++anomalies_found;
            }
            std::cout << "\n";
        }

        previous = reading;
    }

    std::cout << "\n--- summary ---\n"
              << "lines read:      " << line_number << "\n"
              << "readings parsed: " << parsed << "\n"
              << "anomalies found: " << anomalies_found << "\n";
}

}  // namespace

int main(int argc, char** argv) {
    // Read from a replay file if given, otherwise from stdin. Reading stdin
    // lets the program sit at the end of a live feed, e.g. a poller piping the
    // ISS API in line by line.
    if (argc > 1) {
        std::ifstream file(argv[1]);
        if (!file) {
            std::cerr << "error: could not open " << argv[1] << "\n";
            return 1;
        }
        processStream(file);
    } else {
        processStream(std::cin);
    }
    return 0;
}
