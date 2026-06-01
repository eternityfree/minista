// Copyright 2026 MiniSTA Project
// Licensed under the MIT License.
//
// netlist_parser.cpp — Simple netlist format parser implementation.

#include "core/netlist_parser.h"

#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>

namespace minista {
namespace core {

// ============================================================
// Public Methods
// ============================================================

ParseResult NetlistParser::parse_file(const std::string& filename,
                                      StaEngine& engine) {
    ParseResult result;

    std::ifstream file(filename);
    if (!file.is_open()) {
        result.errors.push_back("Cannot open file: " + filename);
        return result;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    return parse_string(content, engine);
}

ParseResult NetlistParser::parse_string(const std::string& content,
                                        StaEngine& engine) {
    ParseResult result;
    std::istringstream stream(content);
    std::string line;
    int line_num = 0;

    while (std::getline(stream, line)) {
        ++line_num;
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Get the keyword (first word)
        std::istringstream line_stream(line);
        std::string keyword;
        line_stream >> keyword;

        bool parsed = false;
        if (keyword == "CLOCK") {
            parsed = parse_clock(line, line_num, engine, result);
        } else if (keyword == "CELL") {
            parsed = parse_cell(line, line_num, engine, result);
        } else if (keyword == "NET") {
            parsed = parse_net(line, line_num, engine, result);
        } else if (keyword == "PORT") {
            parsed = parse_port(line, line_num, engine, result);
        } else {
            result.errors.push_back(
                "Line " + std::to_string(line_num) +
                ": Unknown keyword '" + keyword + "'");
        }

        if (parsed) {
            ++result.lines_parsed;
        }
    }

    result.success = result.errors.empty();
    return result;
}

// ============================================================
// Private Methods — Parsing
// ============================================================

bool NetlistParser::parse_clock(const std::string& line, int line_num,
                                StaEngine& engine, ParseResult& result) {
    // Format: CLOCK <name> period=<ns> skew=<ns>
    std::regex pattern(
        R"(CLOCK\s+(\w+)\s+period=([0-9.]+)(?:\s+skew=([0-9.]+))?)");
    std::smatch match;

    if (!std::regex_match(line, match, pattern)) {
        result.errors.push_back(
            "Line " + std::to_string(line_num) +
            ": Invalid CLOCK syntax. Expected: CLOCK <name> period=<ns> [skew=<ns>]");
        return false;
    }

    std::string name = match[1].str();
    double period = std::stod(match[2].str());
    double skew = match[3].matched ? std::stod(match[3].str()) : 0.0;

    if (!engine.create_clock(name, period, skew)) {
        result.errors.push_back(
            "Line " + std::to_string(line_num) +
            ": Failed to create clock '" + name + "'");
        return false;
    }

    ++result.clocks_created;
    return true;
}

bool NetlistParser::parse_cell(const std::string& line, int line_num,
                               StaEngine& engine, ParseResult& result) {
    // Format: CELL <name> <type> tcq=<ns> tsetup=<ns> thold=<ns>
    std::regex pattern(
        R"(CELL\s+(\w+)\s+(DFF|LATCH)\s+tcq=([0-9.]+)\s+tsetup=([0-9.]+)\s+thold=([0-9.]+))");
    std::smatch match;

    if (!std::regex_match(line, match, pattern)) {
        result.errors.push_back(
            "Line " + std::to_string(line_num) +
            ": Invalid CELL syntax. Expected: CELL <name> <DFF|LATCH> tcq=<ns> tsetup=<ns> thold=<ns>");
        return false;
    }

    std::string name = match[1].str();
    std::string type = match[2].str();
    double tcq = std::stod(match[3].str());
    double tsetup = std::stod(match[4].str());
    double thold = std::stod(match[5].str());

    if (!engine.create_cell(name, type, tcq, tsetup, thold)) {
        result.errors.push_back(
            "Line " + std::to_string(line_num) +
            ": Failed to create cell '" + name + "'");
        return false;
    }

    ++result.cells_created;
    return true;
}

bool NetlistParser::parse_net(const std::string& line, int line_num,
                              StaEngine& engine, ParseResult& result) {
    // Format: NET <name> FROM <cell>/<pin> TO <cell>/<pin> delay=<ns>
    std::regex pattern(
        R"(NET\s+(\w+)\s+FROM\s+(\w+)/(\w+)\s+TO\s+(\w+)/(\w+)\s+delay=([0-9.]+))");
    std::smatch match;

    if (!std::regex_match(line, match, pattern)) {
        result.errors.push_back(
            "Line " + std::to_string(line_num) +
            ": Invalid NET syntax. Expected: NET <name> FROM <cell>/<pin> TO <cell>/<pin> delay=<ns>");
        return false;
    }

    std::string name = match[1].str();
    std::string from_cell = match[2].str();
    std::string from_pin = match[3].str();
    std::string to_cell = match[4].str();
    std::string to_pin = match[5].str();
    double delay = std::stod(match[6].str());

    if (!engine.create_net(name, from_cell, from_pin, to_cell, to_pin, delay)) {
        result.errors.push_back(
            "Line " + std::to_string(line_num) +
            ": Failed to create net '" + name + "'");
        return false;
    }

    ++result.nets_created;
    return true;
}

bool NetlistParser::parse_port(const std::string& line, int line_num,
                               StaEngine& engine, ParseResult& result) {
    // Format: PORT <name> <INPUT|OUTPUT> delay=<ns>
    // Ports are optional and stored as cells with special type
    std::regex pattern(
        R"(PORT\s+(\w+)\s+(INPUT|OUTPUT)\s+delay=([0-9.]+))");
    std::smatch match;

    if (!std::regex_match(line, match, pattern)) {
        result.errors.push_back(
            "Line " + std::to_string(line_num) +
            ": Invalid PORT syntax. Expected: PORT <name> <INPUT|OUTPUT> delay=<ns>");
        return false;
    }

    std::string name = match[1].str();
    std::string direction = match[2].str();
    double delay = std::stod(match[3].str());

    // Store port as a cell with type "PORT_IN" or "PORT_OUT"
    std::string type = (direction == "INPUT") ? "PORT_IN" : "PORT_OUT";
    if (!engine.create_cell(name, type, delay, 0.0, 0.0)) {
        result.errors.push_back(
            "Line " + std::to_string(line_num) +
            ": Failed to create port '" + name + "'");
        return false;
    }

    ++result.ports_created;
    return true;
}

// ============================================================
// Helper Functions
// ============================================================

std::string NetlistParser::trim(const std::string& s) const {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool NetlistParser::starts_with(const std::string& s,
                                const std::string& prefix) const {
    return s.substr(0, prefix.size()) == prefix;
}

std::pair<std::string, std::string> NetlistParser::split_key_value(
    const std::string& s) const {
    auto pos = s.find('=');
    if (pos == std::string::npos) {
        return {s, ""};
    }
    return {s.substr(0, pos), s.substr(pos + 1)};
}

}  // namespace core
}  // namespace minista
