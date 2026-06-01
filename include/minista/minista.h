// Copyright 2026 MiniSTA Project
// Licensed under the MIT License.
//
// minista.h — Main public header for MiniSTA.
//
// Include this header to access all MiniSTA public APIs.
// This file aggregates the core type definitions and provides
// version information for the library.

#ifndef MINISTA_MINISTA_H_
#define MINISTA_MINISTA_H_

// Core type definitions
#include "minista/types.h"

namespace minista {

// ============================================================
// Version Information
// ============================================================
struct Version {
    static constexpr int kMajor = 0;
    static constexpr int kMinor = 1;
    static constexpr int kPatch = 0;

    // Returns version string in "MAJOR.MINOR.PATCH" format.
    static const char* String() {
        return "0.1.0";
    }
};

// ============================================================
// Project Description
// ============================================================
constexpr const char* kProjectName = "MiniSTA";
constexpr const char* kProjectDescription =
    "Lightweight Static Timing Analyzer with Tcl scripting engine";
constexpr const char* kProjectBanner = R"(
 ███╗   ███╗ ██╗ ███╗   ██╗ ██╗ ███████╗ ████████╗  █████╗
 ████╗ ████║ ██║ ████╗  ██║ ██║ ██╔════╝ ╚══██╔══╝ ██╔══██╗
 ██╔████╔██║ ██║ ██╔██╗ ██║ ██║ ███████╗    ██║    ███████║
 ██║╚██╔╝██║ ██║ ██║╚██╗██║ ██║ ╚════██║    ██║    ██╔══██║
 ██║ ╚═╝ ██║ ██║ ██║ ╚████║ ██║ ███████║    ██║    ██║  ██║
 ╚═╝     ╚═╝ ╚═╝ ╚═╝  ╚═══╝ ╚═╝ ╚══════╝    ╚═╝    ╚═╝  ╚═╝
                                          by luozhiyun

 Lightweight Static Timing Analyzer v0.1.0
 Type 'help' for available commands, 'exit' to quit.
)";

}  // namespace minista

#endif  // MINISTA_MINISTA_H_
