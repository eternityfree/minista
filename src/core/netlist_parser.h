// Copyright 2026 MiniSTA Project
// Licensed under the MIT License.
//
// netlist_parser.h — Simple netlist format parser.
//
// Parses .net files and populates the StaEngine with design data.

#ifndef MINISTA_CORE_NETLIST_PARSER_H_
#define MINISTA_CORE_NETLIST_PARSER_H_

#include <string>
#include <vector>

#include "core/sta_engine.h"

namespace minista {
namespace core {

// ============================================================
// Parse Result
// ============================================================

struct ParseResult {
    bool success = false;
    int lines_parsed = 0;
    int clocks_created = 0;
    int cells_created = 0;
    int nets_created = 0;
    int ports_created = 0;
    std::vector<std::string> errors;
};

// ============================================================
// Netlist Parser
// ============================================================

class NetlistParser {
public:
    NetlistParser() = default;

    // Parse a .net file and populate the engine
    ParseResult parse_file(const std::string& filename, StaEngine& engine);

    // Parse a string (for testing)
    ParseResult parse_string(const std::string& content, StaEngine& engine);

private:
    // Parse individual line types
    bool parse_clock(const std::string& line, int line_num,
                     StaEngine& engine, ParseResult& result);
    bool parse_cell(const std::string& line, int line_num,
                    StaEngine& engine, ParseResult& result);
    bool parse_net(const std::string& line, int line_num,
                   StaEngine& engine, ParseResult& result);
    bool parse_port(const std::string& line, int line_num,
                    StaEngine& engine, ParseResult& result);

    // Helper functions
    std::string trim(const std::string& s) const;
    bool starts_with(const std::string& s, const std::string& prefix) const;
    std::pair<std::string, std::string> split_key_value(
        const std::string& s) const;
};

}  // namespace core
}  // namespace minista

#endif  // MINISTA_CORE_NETLIST_PARSER_H_
