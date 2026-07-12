#ifndef TELEMETRY_PARSER_H
#define TELEMETRY_PARSER_H

#include <optional>
#include <string>

#include "TelemetryReading.h"

// Parses one line of ISS telemetry JSON into a TelemetryReading.
//
// Interface contract:
//   Input : one line of text expected to contain a flat JSON object with the
//           keys timestamp, latitude, longitude, altitude, velocity.
//   Output: a populated TelemetryReading on success; std::nullopt if the line
//           is empty, not an object, or missing a required numeric field.
//
// The parser is intentionally dependency-free (no external JSON library) so the
// project builds with nothing but a C++17 compiler. It handles the flat,
// well-formed objects the ISS API emits; it is not a general-purpose JSON
// parser and does not attempt to.
class TelemetryParser {
public:
    std::optional<TelemetryReading> parse(const std::string& json_line) const;

private:
    // Extracts a numeric value for `key` from `json`. Returns nullopt if the
    // key is absent or its value is not a parseable number.
    std::optional<double> extractNumber(const std::string& json,
                                        const std::string& key) const;
};

#endif  // TELEMETRY_PARSER_H
