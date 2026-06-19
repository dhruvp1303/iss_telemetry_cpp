#include "TelemetryParser.h"

#include <cctype>
#include <cstdlib>
#include <string>

std::optional<double> TelemetryParser::extractNumber(const std::string& json,
                                                     const std::string& key) const {
    // Find the quoted key, e.g. "latitude".
    const std::string quoted_key = "\"" + key + "\"";
    std::size_t pos = json.find(quoted_key);
    if (pos == std::string::npos) {
        return std::nullopt;
    }

    // Advance past the key to the ':' separator.
    pos = json.find(':', pos + quoted_key.size());
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    ++pos;  // step past ':'

    // Skip whitespace and an optional opening quote (the API sometimes returns
    // numbers as JSON strings).
    while (pos < json.size() &&
           (std::isspace(static_cast<unsigned char>(json[pos])) || json[pos] == '"')) {
        ++pos;
    }

    // Collect the numeric token: digits, sign, decimal point, exponent.
    const std::size_t start = pos;
    while (pos < json.size()) {
        const char c = json[pos];
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '-' || c == '+' ||
            c == '.' || c == 'e' || c == 'E') {
            ++pos;
        } else {
            break;
        }
    }

    if (pos == start) {
        return std::nullopt;  // no numeric characters found
    }

    const std::string token = json.substr(start, pos - start);
    try {
        std::size_t consumed = 0;
        const double value = std::stod(token, &consumed);
        if (consumed == 0) {
            return std::nullopt;
        }
        return value;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<TelemetryReading> TelemetryParser::parse(
    const std::string& json_line) const {
    // Reject obviously non-object input early.
    if (json_line.find('{') == std::string::npos) {
        return std::nullopt;
    }

    const auto timestamp = extractNumber(json_line, "timestamp");
    const auto latitude = extractNumber(json_line, "latitude");
    const auto longitude = extractNumber(json_line, "longitude");
    const auto altitude = extractNumber(json_line, "altitude");
    const auto velocity = extractNumber(json_line, "velocity");

    // Every required field must be present and numeric.
    if (!timestamp || !latitude || !longitude || !altitude || !velocity) {
        return std::nullopt;
    }

    TelemetryReading reading;
    reading.timestamp = static_cast<std::int64_t>(*timestamp);
    reading.latitude = *latitude;
    reading.longitude = *longitude;
    reading.altitude_km = *altitude;
    reading.velocity_kmh = *velocity;
    return reading;
}
