#ifndef TELEMETRY_PARSER_H
#define TELEMETRY_PARSER_H

#include <optional>
#include <string>

#include "TelemetryReading.hpp"

class TelemetryParser {
public:
    std::optional<TelemetryReading> parse(const std::string& json_line) const;

private:
    
    std::optional<double> extractNumber(const std::string& json,
                                        const std::string& key) const;
};

#endif  
