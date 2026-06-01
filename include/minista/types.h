// Copyright 2026 MiniSTA Project
// Licensed under the MIT License.
//
// types.h — Core type definitions for MiniSTA.
//
// This file defines enumerations and fundamental types used across
// the MiniSTA static timing analysis engine.

#ifndef MINISTA_TYPES_H_
#define MINISTA_TYPES_H_

#include <cstdint>
#include <string>

namespace minista {

// ============================================================
// Cell Type Enumeration
// ============================================================
// Represents the type of a sequential element in the design.
enum class CellType : uint8_t {
    kDff = 0,    // D Flip-Flop
    kLatch = 1,  // Latch
    kUnknown = 255,
};

// ============================================================
// Path Type Enumeration
// ============================================================
// Represents the type of timing check being performed.
enum class PathType : uint8_t {
    kSetup = 0,  // Setup (max delay) check
    kHold = 1,   // Hold (min delay) check
};

// ============================================================
// Timing Result
// ============================================================
// Represents the outcome of a timing check.
enum class TimingResult : uint8_t {
    kPass = 0,   // Slack >= 0
    kFail = 1,   // Slack < 0
};

// ============================================================
// Pin Direction
// ============================================================
// Represents the direction of a cell pin.
enum class PinDirection : uint8_t {
    kInput = 0,
    kOutput = 1,
};

// ============================================================
// Helper Functions
// ============================================================

// Convert CellType to human-readable string.
inline const char* CellTypeToString(CellType type) {
    switch (type) {
        case CellType::kDff:    return "DFF";
        case CellType::kLatch:  return "LATCH";
        default:                return "UNKNOWN";
    }
}

// Convert PathType to human-readable string.
inline const char* PathTypeToString(PathType type) {
    switch (type) {
        case PathType::kSetup: return "Setup";
        case PathType::kHold:  return "Hold";
        default:               return "Unknown";
    }
}

// Convert TimingResult to human-readable string.
inline const char* TimingResultToString(TimingResult result) {
    switch (result) {
        case TimingResult::kPass: return "PASS";
        case TimingResult::kFail: return "FAIL";
        default:                  return "UNKNOWN";
    }
}

}  // namespace minista

#endif  // MINISTA_TYPES_H_
