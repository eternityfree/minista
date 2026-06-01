// Copyright 2026 MiniSTA Project
// Licensed under the MIT License.
//
// sta_engine.h — Core STA calculation engine.
//
// This file defines the core timing analysis engine that performs
// Setup and Hold slack calculations. It's designed to be independent
// of the scripting interface (Tcl/Python).

#ifndef MINISTA_CORE_STA_ENGINE_H_
#define MINISTA_CORE_STA_ENGINE_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace minista {
namespace core {

// ============================================================
// Data Structures
// ============================================================

struct Clock {
    std::string name;
    double period = 0.0;    // ns
    double skew = 0.0;      // ns
};

struct Cell {
    std::string name;
    std::string type = "DFF";  // DFF or LATCH
    double tcq = 0.0;          // Clock-to-Q delay (ns)
    double tsetup = 0.0;       // Setup time (ns)
    double thold = 0.0;        // Hold time (ns)
};

struct Net {
    std::string name;
    std::string from_cell;
    std::string from_pin;
    std::string to_cell;
    std::string to_pin;
    double delay = 0.0;        // Combinational delay (ns)
};

struct TimingResult {
    double data_arrival = 0.0;
    double data_required = 0.0;
    double slack = 0.0;
    bool pass = false;
    std::string path_type;  // "setup" or "hold"
};

// ============================================================
// STA Engine
// ============================================================

class StaEngine {
public:
    StaEngine() = default;

    // Object creation
    bool create_clock(const std::string& name, double period, double skew = 0.0);
    bool create_cell(const std::string& name, const std::string& type,
                     double tcq, double tsetup, double thold);
    bool create_net(const std::string& name,
                    const std::string& from_cell, const std::string& from_pin,
                    const std::string& to_cell, const std::string& to_pin,
                    double delay);

    // Object queries
    std::optional<Clock> get_clock(const std::string& name) const;
    std::optional<Cell> get_cell(const std::string& name) const;
    std::optional<Net> get_net(const std::string& name) const;

    std::vector<std::string> get_all_clocks() const;
    std::vector<std::string> get_all_cells() const;
    std::vector<std::string> get_all_nets() const;

    // Timing analysis
    TimingResult check_setup(const std::string& from_cell,
                             const std::string& to_cell) const;
    TimingResult check_hold(const std::string& from_cell,
                            const std::string& to_cell) const;

    // Report
    std::string report_timing(const std::string& from_cell,
                              const std::string& to_cell,
                              bool check_setup = true,
                              bool check_hold = true) const;

    // Clear all data
    void clear();

private:
    std::unordered_map<std::string, Clock> clocks_;
    std::unordered_map<std::string, Cell> cells_;
    std::unordered_map<std::string, Net> nets_;

    // Helper: find net connecting two cells
    std::optional<Net> find_net(const std::string& from_cell,
                                const std::string& to_cell) const;
};

}  // namespace core
}  // namespace minista

#endif  // MINISTA_CORE_STA_ENGINE_H_
