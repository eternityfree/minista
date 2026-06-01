// Copyright 2026 MiniSTA Project
// Licensed under the MIT License.
//
// sta_engine.cpp — Core STA calculation engine implementation.

#include "core/sta_engine.h"

#include <sstream>
#include <iomanip>
#include <cmath>

namespace minista {
namespace core {

// ============================================================
// Object Creation
// ============================================================

bool StaEngine::create_clock(const std::string& name, double period, double skew) {
    if (period <= 0) return false;
    clocks_[name] = Clock{name, period, skew};
    return true;
}

bool StaEngine::create_cell(const std::string& name, const std::string& type,
                            double tcq, double tsetup, double thold) {
    cells_[name] = Cell{name, type, tcq, tsetup, thold};
    return true;
}

bool StaEngine::create_net(const std::string& name,
                           const std::string& from_cell, const std::string& from_pin,
                           const std::string& to_cell, const std::string& to_pin,
                           double delay) {
    nets_[name] = Net{name, from_cell, from_pin, to_cell, to_pin, delay};
    return true;
}

// ============================================================
// Object Queries
// ============================================================

std::optional<Clock> StaEngine::get_clock(const std::string& name) const {
    auto it = clocks_.find(name);
    if (it != clocks_.end()) return it->second;
    return std::nullopt;
}

std::optional<Cell> StaEngine::get_cell(const std::string& name) const {
    auto it = cells_.find(name);
    if (it != cells_.end()) return it->second;
    return std::nullopt;
}

std::optional<Net> StaEngine::get_net(const std::string& name) const {
    auto it = nets_.find(name);
    if (it != nets_.end()) return it->second;
    return std::nullopt;
}

std::vector<std::string> StaEngine::get_all_clocks() const {
    std::vector<std::string> result;
    for (const auto& [name, _] : clocks_) result.push_back(name);
    return result;
}

std::vector<std::string> StaEngine::get_all_cells() const {
    std::vector<std::string> result;
    for (const auto& [name, _] : cells_) result.push_back(name);
    return result;
}

std::vector<std::string> StaEngine::get_all_nets() const {
    std::vector<std::string> result;
    for (const auto& [name, _] : nets_) result.push_back(name);
    return result;
}

// ============================================================
// Helper
// ============================================================

std::optional<Net> StaEngine::find_net(const std::string& from_cell,
                                       const std::string& to_cell) const {
    for (const auto& [_, net] : nets_) {
        if (net.from_cell == from_cell && net.to_cell == to_cell) {
            return net;
        }
    }
    return std::nullopt;
}

// ============================================================
// Timing Analysis
// ============================================================

TimingResult StaEngine::check_setup(const std::string& from_cell_name,
                                    const std::string& to_cell_name) const {
    TimingResult result;
    result.path_type = "setup";

    auto from_cell = get_cell(from_cell_name);
    auto to_cell = get_cell(to_cell_name);
    auto net = find_net(from_cell_name, to_cell_name);

    if (!from_cell || !to_cell || !net) {
        result.pass = false;
        return result;
    }

    // Find clock (use first clock for simplicity)
    double clock_period = 10.0;  // default
    double clock_skew = 0.0;
    if (!clocks_.empty()) {
        const auto& clk = clocks_.begin()->second;
        clock_period = clk.period;
        clock_skew = clk.skew;
    }

    // Setup Slack = (T_clk + T_skew) - (T_cq + T_comb + T_setup)
    result.data_arrival = from_cell->tcq + net->delay;
    result.data_required = clock_period + clock_skew - to_cell->tsetup;
    result.slack = result.data_required - result.data_arrival;
    result.pass = (result.slack >= 0);

    return result;
}

TimingResult StaEngine::check_hold(const std::string& from_cell_name,
                                   const std::string& to_cell_name) const {
    TimingResult result;
    result.path_type = "hold";

    auto from_cell = get_cell(from_cell_name);
    auto to_cell = get_cell(to_cell_name);
    auto net = find_net(from_cell_name, to_cell_name);

    if (!from_cell || !to_cell || !net) {
        result.pass = false;
        return result;
    }

    // Find clock
    double clock_skew = 0.0;
    if (!clocks_.empty()) {
        clock_skew = clocks_.begin()->second.skew;
    }

    // Hold Slack = (T_cq + T_comb) - (T_hold - T_skew)
    result.data_arrival = from_cell->tcq + net->delay;
    result.data_required = to_cell->thold - clock_skew;
    result.slack = result.data_arrival - result.data_required;
    result.pass = (result.slack >= 0);

    return result;
}

// ============================================================
// Report
// ============================================================

std::string StaEngine::report_timing(const std::string& from_cell,
                                     const std::string& to_cell,
                                     bool check_setup_flag,
                                     bool check_hold_flag) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);

    oss << "================================================================\n";
    oss << "                     Timing Report\n";
    oss << "================================================================\n\n";
    oss << "Startpoint: " << from_cell << "\n";
    oss << "Endpoint:   " << to_cell << "\n\n";

    if (check_setup_flag) {
        auto setup = check_setup(from_cell, to_cell);
        oss << "Setup Check:\n";
        oss << "  Data Arrival:    " << setup.data_arrival << " ns\n";
        oss << "  Data Required:   " << setup.data_required << " ns\n";
        oss << "  Slack:           " << setup.slack << " ns"
            << (setup.pass ? " (PASS)" : " (FAIL)") << "\n\n";
    }

    if (check_hold_flag) {
        auto hold = check_hold(from_cell, to_cell);
        oss << "Hold Check:\n";
        oss << "  Data Arrival:    " << hold.data_arrival << " ns\n";
        oss << "  Data Required:   " << hold.data_required << " ns\n";
        oss << "  Slack:           " << hold.slack << " ns"
            << (hold.pass ? " (PASS)" : " (FAIL)") << "\n\n";
    }

    oss << "================================================================\n";
    return oss.str();
}

void StaEngine::clear() {
    clocks_.clear();
    cells_.clear();
    nets_.clear();
}

}  // namespace core
}  // namespace minista
