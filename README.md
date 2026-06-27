# ISS Real-Time Telemetry Processor (C++)

A C++17 application that ingests a continuous position-telemetry stream (the
public ISS feed from [wheretheiss.at](https://wheretheiss.at/w/developer)),
computes derived metrics in real time, and flags readings that fall outside a
configured operating envelope. Built and tested on Linux with CMake and
GoogleTest.

The program reads one telemetry sample per line, so it processes each message in
bounded work before moving to the next — it can sit at the end of a live feed
(stdin) or replay a recorded one (a file).

## Architecture

The pipeline is split into single-purpose modules, each with a defined
interface contract:

| Module | Responsibility |
|---|---|
| `TelemetryParser` | One line of ISS JSON → a typed `TelemetryReading` (or rejection) |
| `MetricsEngine` | Ground-track distance (haversine) and velocity delta between consecutive readings |
| `AnomalyDetector` | Checks each reading and its metrics against configurable thresholds |
| `main` (ingest loop) | parse → compute → detect → report, once per sample |

## Requirements verified by the test suite

| # | Requirement | Test |
|---|---|---|
| R1 | A well-formed telemetry line parses into the correct typed fields | `TelemetryParser.ParsesWellFormedLine` |
| R2 | Numeric values encoded as JSON strings are accepted | `TelemetryParser.HandlesValuesEncodedAsStrings` |
| R3 | A line missing a required field is rejected | `TelemetryParser.RejectsMissingField` |
| R4 | Non-JSON / empty input is rejected | `TelemetryParser.RejectsGarbageInput` |
| R5 | Ground-track distance is zero for identical points | `MetricsEngine.GroundTrackZeroForSamePoint` |
| R6 | Ground-track distance matches a known reference (London–Paris ≈ 343 km) | `MetricsEngine.GroundTrackKnownDistance` |
| R7 | Ground-track distance is symmetric | `MetricsEngine.GroundTrackIsSymmetric` |
| R8 | Velocity delta and elapsed time are computed correctly | `MetricsEngine.ComputesVelocityDeltaAndElapsed` |
| R9 | A nominal reading produces no anomalies | `AnomalyDetector.NominalReadingHasNoAnomalies` |
| R10 | Altitude above the ceiling is flagged | `AnomalyDetector.FlagsAltitudeAboveCeiling` |
| R11 | An implausible velocity jump is flagged | `AnomalyDetector.FlagsVelocityJump` |
| R12 | Multiple simultaneous violations are all reported | `AnomalyDetector.ReportsMultipleViolationsAtOnce` |
| R13 | Custom thresholds override defaults | `AnomalyDetector.RespectsCustomThresholds` |

## Build

Requires a C++17 compiler and CMake ≥ 3.14. GoogleTest is fetched automatically
at configure time.

```bash
cmake -B build
cmake --build build
```

## Run

Replay the bundled sample feed:

```bash
./build/telemetry data/sample_feed.jsonl
```

Or pipe a live feed in line by line (any source that emits one JSON object per
line works):

```bash
cat data/sample_feed.jsonl | ./build/telemetry
```

Example output:

```
[1700000000] lat=40.7128 lon=-74.0060 alt=408.5km  (baseline)
[1700000005] track=51.1km  dV=+2.3km/h  OK
...
[1700000030] track=51.4km  dV=+2.2km/h  ANOMALY: [altitude_high: altitude above expected maximum]
```

## Test

```bash
cd build && ctest --output-on-failure
```

or run the test binary directly:

```bash
./build/telemetry_tests
```

## Notes

The parser is intentionally dependency-free (no external JSON library) so the
project builds with nothing but a compiler and CMake. It targets the flat,
well-formed objects the ISS API emits rather than being a general-purpose JSON
parser. Anomaly thresholds default to brackets around the real ISS operating
envelope (~370–460 km altitude, ~27,500 km/h) and are configurable via
`AnomalyThresholds`.
