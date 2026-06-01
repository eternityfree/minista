// Copyright 2026 MiniSTA Project
// Licensed under the MIT License.
//
// bindings.cpp — pybind11 Python bindings for MiniSTA.
//
// This file exposes the core STA engine to Python using pybind11.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "core/sta_engine.h"
#include "core/netlist_parser.h"

namespace py = pybind11;
using namespace minista::core;

PYBIND11_MODULE(minista, m) {
    m.doc() = R"(
MiniSTA — Lightweight Static Timing Analyzer

A Python binding for the MiniSTA STA engine.

Example usage:
    import minista

    engine = minista.StaEngine()
    engine.create_clock("clk", 10.0, 0.1)
    engine.create_cell("reg1", "DFF", 0.5, 0.2, 0.1)
    engine.create_cell("reg2", "DFF", 0.5, 0.2, 0.1)
    engine.create_net("net1", "reg1", "Q", "reg2", "D", 3.0)

    result = engine.check_setup("reg1", "reg2")
    print(f"Setup Slack: {result.slack} ns")
)";

    // ============================================================
    // TimingResult
    // ============================================================
    py::class_<TimingResult>(m, "TimingResult")
        .def_readonly("data_arrival", &TimingResult::data_arrival)
        .def_readonly("data_required", &TimingResult::data_required)
        .def_readonly("slack", &TimingResult::slack)
        .def_readonly("is_pass", &TimingResult::pass)  // 'pass' is reserved in Python
        .def_readonly("path_type", &TimingResult::path_type)
        .def("__repr__", [](const TimingResult& r) {
            return "<TimingResult type='" + r.path_type +
                   "' slack=" + std::to_string(r.slack) +
                   " " + (r.pass ? "PASS" : "FAIL") + ">";
        });

    // ============================================================
    // Clock
    // ============================================================
    py::class_<Clock>(m, "Clock")
        .def_readonly("name", &Clock::name)
        .def_readonly("period", &Clock::period)
        .def_readonly("skew", &Clock::skew)
        .def("__repr__", [](const Clock& c) {
            return "<Clock name='" + c.name +
                   "' period=" + std::to_string(c.period) + ">";
        });

    // ============================================================
    // Cell
    // ============================================================
    py::class_<Cell>(m, "Cell")
        .def_readonly("name", &Cell::name)
        .def_readonly("type", &Cell::type)
        .def_readonly("tcq", &Cell::tcq)
        .def_readonly("tsetup", &Cell::tsetup)
        .def_readonly("thold", &Cell::thold)
        .def("__repr__", [](const Cell& c) {
            return "<Cell name='" + c.name + "' type='" + c.type + "'>";
        });

    // ============================================================
    // Net
    // ============================================================
    py::class_<Net>(m, "Net")
        .def_readonly("name", &Net::name)
        .def_readonly("from_cell", &Net::from_cell)
        .def_readonly("from_pin", &Net::from_pin)
        .def_readonly("to_cell", &Net::to_cell)
        .def_readonly("to_pin", &Net::to_pin)
        .def_readonly("delay", &Net::delay)
        .def("__repr__", [](const Net& n) {
            return "<Net name='" + n.name + "' delay=" +
                   std::to_string(n.delay) + ">";
        });

    // ============================================================
    // StaEngine
    // ============================================================
    py::class_<StaEngine>(m, "StaEngine")
        .def(py::init<>(), R"(
Create a new STA engine instance.

Returns:
    StaEngine: A new timing analysis engine.
)")

        // Object creation
        .def("create_clock", &StaEngine::create_clock,
             py::arg("name"), py::arg("period"), py::arg("skew") = 0.0,
             R"(
Create a clock definition.

Args:
    name (str): Clock name.
    period (float): Clock period in ns.
    skew (float): Clock skew in ns (default: 0.0).

Returns:
    bool: True if successful.
)")

        .def("create_cell", &StaEngine::create_cell,
             py::arg("name"), py::arg("type") = "DFF",
             py::arg("tcq") = 0.0, py::arg("tsetup") = 0.0, py::arg("thold") = 0.0,
             R"(
Create a cell (register) definition.

Args:
    name (str): Cell name.
    type (str): Cell type ('DFF' or 'LATCH').
    tcq (float): Clock-to-Q delay in ns.
    tsetup (float): Setup time in ns.
    thold (float): Hold time in ns.

Returns:
    bool: True if successful.
)")

        .def("create_net", &StaEngine::create_net,
             py::arg("name"),
             py::arg("from_cell"), py::arg("from_pin"),
             py::arg("to_cell"), py::arg("to_pin"),
             py::arg("delay") = 0.0,
             R"(
Create a net (connection) between cells.

Args:
    name (str): Net name.
    from_cell (str): Source cell name.
    from_pin (str): Source pin name (e.g., 'Q').
    to_cell (str): Destination cell name.
    to_pin (str): Destination pin name (e.g., 'D').
    delay (float): Combinational delay in ns.

Returns:
    bool: True if successful.
)")

        // Object queries
        .def("get_clock", &StaEngine::get_clock, py::arg("name"),
             "Get a clock by name. Returns None if not found.")
        .def("get_cell", &StaEngine::get_cell, py::arg("name"),
             "Get a cell by name. Returns None if not found.")
        .def("get_net", &StaEngine::get_net, py::arg("name"),
             "Get a net by name. Returns None if not found.")

        .def("get_all_clocks", &StaEngine::get_all_clocks,
             "Get list of all clock names.")
        .def("get_all_cells", &StaEngine::get_all_cells,
             "Get list of all cell names.")
        .def("get_all_nets", &StaEngine::get_all_nets,
             "Get list of all net names.")

        // Timing analysis
        .def("check_setup", &StaEngine::check_setup,
             py::arg("from_cell"), py::arg("to_cell"),
             R"(
Check setup timing between two cells.

Args:
    from_cell (str): Startpoint cell name.
    to_cell (str): Endpoint cell name.

Returns:
    TimingResult: Setup timing analysis result.
)")

        .def("check_hold", &StaEngine::check_hold,
             py::arg("from_cell"), py::arg("to_cell"),
             R"(
Check hold timing between two cells.

Args:
    from_cell (str): Startpoint cell name.
    to_cell (str): Endpoint cell name.

Returns:
    TimingResult: Hold timing analysis result.
)")

        .def("report_timing", &StaEngine::report_timing,
             py::arg("from_cell"), py::arg("to_cell"),
             py::arg("check_setup") = true, py::arg("check_hold") = true,
             R"(
Generate a timing report.

Args:
    from_cell (str): Startpoint cell name.
    to_cell (str): Endpoint cell name.
    check_setup (bool): Include setup check (default: True).
    check_hold (bool): Include hold check (default: True).

Returns:
    str: Formatted timing report.
)")

        // Utility
        .def("clear", &StaEngine::clear, "Clear all design data.");

    // ============================================================
    // ParseResult
    // ============================================================
    py::class_<ParseResult>(m, "ParseResult")
        .def_readonly("success", &ParseResult::success)
        .def_readonly("lines_parsed", &ParseResult::lines_parsed)
        .def_readonly("clocks_created", &ParseResult::clocks_created)
        .def_readonly("cells_created", &ParseResult::cells_created)
        .def_readonly("nets_created", &ParseResult::nets_created)
        .def_readonly("ports_created", &ParseResult::ports_created)
        .def_readonly("errors", &ParseResult::errors)
        .def("__repr__", [](const ParseResult& r) {
            return "<ParseResult success=" + std::string(r.success ? "True" : "False") +
                   " clocks=" + std::to_string(r.clocks_created) +
                   " cells=" + std::to_string(r.cells_created) +
                   " nets=" + std::to_string(r.nets_created) + ">";
        });

    // ============================================================
    // NetlistParser
    // ============================================================
    py::class_<NetlistParser>(m, "NetlistParser")
        .def(py::init<>(), R"(
Create a new netlist parser instance.
)")

        .def("parse_file", &NetlistParser::parse_file,
             py::arg("filename"), py::arg("engine"),
             R"(
Parse a .net file and populate the engine.

Args:
    filename (str): Path to the .net file.
    engine (StaEngine): The STA engine to populate.

Returns:
    ParseResult: Parse result with statistics and errors.
)")

        .def("parse_string", &NetlistParser::parse_string,
             py::arg("content"), py::arg("engine"),
             R"(
Parse a netlist string and populate the engine.

Args:
    content (str): Netlist content string.
    engine (StaEngine): The STA engine to populate.

Returns:
    ParseResult: Parse result with statistics and errors.
)");
}
