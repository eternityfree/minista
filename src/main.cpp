// Copyright 2026 MiniSTA Project
// Licensed under the MIT License.
//
// main.cpp — MiniSTA entry point.
//
// This file implements the interactive Tcl shell for MiniSTA.
// It creates a Tcl interpreter, registers custom commands,
// and runs a read-eval-print loop (REPL) with a "tcl>" prompt.

#include <tcl.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <readline/readline.h>
#include <readline/history.h>

#include "minista/minista.h"
#include "core/sta_engine.h"
#include "core/netlist_parser.h"

namespace {

// Global STA engine (shared across all Tcl commands)
minista::core::StaEngine g_engine;
minista::core::NetlistParser g_parser;

// ============================================================
// Helper: Get string argument
// ============================================================
std::string GetArg(Tcl_Interp* interp, Tcl_Obj* const objv[], int index) {
    return Tcl_GetString(objv[index]);
}

// ============================================================
// Helper: Get double argument
// ============================================================
bool GetDouble(Tcl_Interp* interp, Tcl_Obj* const objv[], int index,
               double& value) {
    return Tcl_GetDoubleFromObj(interp, objv[index], &value) == TCL_OK;
}

// ============================================================
// Tcl Command: hello
// ============================================================
int HelloCmd(ClientData /*client_data*/, Tcl_Interp* interp,
             int objc, Tcl_Obj* const objv[]) {
    if (objc > 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "?name?");
        return TCL_ERROR;
    }

    std::string name = "World";
    if (objc == 2) {
        name = GetArg(interp, objv, 1);
    }

    std::string result = "Hello, " + name + "!";
    Tcl_SetObjResult(interp, Tcl_NewStringObj(result.c_str(), -1));
    return TCL_OK;
}

// ============================================================
// Tcl Command: minista_version
// ============================================================
int MinistaVersionCmd(ClientData /*client_data*/, Tcl_Interp* interp,
                      int objc, Tcl_Obj* const objv[]) {
    if (objc != 1) {
        Tcl_WrongNumArgs(interp, 1, objv, nullptr);
        return TCL_ERROR;
    }

    Tcl_SetObjResult(interp,
                     Tcl_NewStringObj(minista::Version::String(), -1));
    return TCL_OK;
}

// ============================================================
// Tcl Command: create_clock
// Usage: create_clock -name <name> -period <ns> [-skew <ns>]
// ============================================================
int CreateClockCmd(ClientData /*client_data*/, Tcl_Interp* interp,
                   int objc, Tcl_Obj* const objv[]) {
    std::string name;
    double period = 0.0;
    double skew = 0.0;

    // Parse named arguments
    for (int i = 1; i < objc; i += 2) {
        if (i + 1 >= objc) {
            Tcl_SetResult(interp, (char*)"Missing value for option", TCL_STATIC);
            return TCL_ERROR;
        }

        std::string opt = GetArg(interp, objv, i);
        if (opt == "-name") {
            name = GetArg(interp, objv, i + 1);
        } else if (opt == "-period") {
            if (!GetDouble(interp, objv, i + 1, period)) return TCL_ERROR;
        } else if (opt == "-skew") {
            if (!GetDouble(interp, objv, i + 1, skew)) return TCL_ERROR;
        } else {
            std::string err = "Unknown option: " + opt;
            Tcl_SetResult(interp, (char*)err.c_str(), TCL_VOLATILE);
            return TCL_ERROR;
        }
    }

    if (name.empty()) {
        Tcl_SetResult(interp, (char*)"Missing required option: -name", TCL_STATIC);
        return TCL_ERROR;
    }
    if (period <= 0) {
        Tcl_SetResult(interp, (char*)"Period must be positive", TCL_STATIC);
        return TCL_ERROR;
    }

    if (!g_engine.create_clock(name, period, skew)) {
        Tcl_SetResult(interp, (char*)"Failed to create clock", TCL_STATIC);
        return TCL_ERROR;
    }

    return TCL_OK;
}

// ============================================================
// Tcl Command: create_cell
// Usage: create_cell -name <name> -type <DFF|LATCH> -tcq <ns> -tsetup <ns> -thold <ns>
// ============================================================
int CreateCellCmd(ClientData /*client_data*/, Tcl_Interp* interp,
                  int objc, Tcl_Obj* const objv[]) {
    std::string name;
    std::string type = "DFF";
    double tcq = 0.0, tsetup = 0.0, thold = 0.0;

    for (int i = 1; i < objc; i += 2) {
        if (i + 1 >= objc) {
            Tcl_SetResult(interp, (char*)"Missing value for option", TCL_STATIC);
            return TCL_ERROR;
        }

        std::string opt = GetArg(interp, objv, i);
        if (opt == "-name") {
            name = GetArg(interp, objv, i + 1);
        } else if (opt == "-type") {
            type = GetArg(interp, objv, i + 1);
        } else if (opt == "-tcq") {
            if (!GetDouble(interp, objv, i + 1, tcq)) return TCL_ERROR;
        } else if (opt == "-tsetup") {
            if (!GetDouble(interp, objv, i + 1, tsetup)) return TCL_ERROR;
        } else if (opt == "-thold") {
            if (!GetDouble(interp, objv, i + 1, thold)) return TCL_ERROR;
        } else {
            std::string err = "Unknown option: " + opt;
            Tcl_SetResult(interp, (char*)err.c_str(), TCL_VOLATILE);
            return TCL_ERROR;
        }
    }

    if (name.empty()) {
        Tcl_SetResult(interp, (char*)"Missing required option: -name", TCL_STATIC);
        return TCL_ERROR;
    }

    if (!g_engine.create_cell(name, type, tcq, tsetup, thold)) {
        Tcl_SetResult(interp, (char*)"Failed to create cell", TCL_STATIC);
        return TCL_ERROR;
    }

    return TCL_OK;
}

// ============================================================
// Tcl Command: create_net
// Usage: create_net -name <name> -from <cell>/<pin> -to <cell>/<pin> [-delay <ns>]
// ============================================================
int CreateNetCmd(ClientData /*client_data*/, Tcl_Interp* interp,
                 int objc, Tcl_Obj* const objv[]) {
    std::string name, from, to;
    double delay = 0.0;

    for (int i = 1; i < objc; i += 2) {
        if (i + 1 >= objc) {
            Tcl_SetResult(interp, (char*)"Missing value for option", TCL_STATIC);
            return TCL_ERROR;
        }

        std::string opt = GetArg(interp, objv, i);
        if (opt == "-name") {
            name = GetArg(interp, objv, i + 1);
        } else if (opt == "-from") {
            from = GetArg(interp, objv, i + 1);
        } else if (opt == "-to") {
            to = GetArg(interp, objv, i + 1);
        } else if (opt == "-delay") {
            if (!GetDouble(interp, objv, i + 1, delay)) return TCL_ERROR;
        } else {
            std::string err = "Unknown option: " + opt;
            Tcl_SetResult(interp, (char*)err.c_str(), TCL_VOLATILE);
            return TCL_ERROR;
        }
    }

    if (name.empty() || from.empty() || to.empty()) {
        Tcl_SetResult(interp, (char*)"Missing required options: -name, -from, -to", TCL_STATIC);
        return TCL_ERROR;
    }

    // Parse "cell/pin" format
    auto from_pos = from.find('/');
    auto to_pos = to.find('/');
    if (from_pos == std::string::npos || to_pos == std::string::npos) {
        Tcl_SetResult(interp, (char*)"Invalid format. Use: cell/pin", TCL_STATIC);
        return TCL_ERROR;
    }

    std::string from_cell = from.substr(0, from_pos);
    std::string from_pin = from.substr(from_pos + 1);
    std::string to_cell = to.substr(0, to_pos);
    std::string to_pin = to.substr(to_pos + 1);

    if (!g_engine.create_net(name, from_cell, from_pin, to_cell, to_pin, delay)) {
        Tcl_SetResult(interp, (char*)"Failed to create net", TCL_STATIC);
        return TCL_ERROR;
    }

    return TCL_OK;
}

// ============================================================
// Tcl Command: report_timing
// Usage: report_timing -from <cell> -to <cell> [-setup] [-hold]
// ============================================================
int ReportTimingCmd(ClientData /*client_data*/, Tcl_Interp* interp,
                    int objc, Tcl_Obj* const objv[]) {
    std::string from, to;
    bool check_setup = true;
    bool check_hold = true;

    for (int i = 1; i < objc; ++i) {
        std::string opt = GetArg(interp, objv, i);
        if (opt == "-from" && i + 1 < objc) {
            from = GetArg(interp, objv, ++i);
        } else if (opt == "-to" && i + 1 < objc) {
            to = GetArg(interp, objv, ++i);
        } else if (opt == "-setup") {
            check_setup = true;
            check_hold = false;
        } else if (opt == "-hold") {
            check_setup = false;
            check_hold = true;
        } else {
            // Ignore unknown options for now
        }
    }

    if (from.empty() || to.empty()) {
        Tcl_SetResult(interp, (char*)"Missing required options: -from, -to", TCL_STATIC);
        return TCL_ERROR;
    }

    std::string report = g_engine.report_timing(from, to, check_setup, check_hold);
    Tcl_SetObjResult(interp, Tcl_NewStringObj(report.c_str(), -1));
    return TCL_OK;
}

// ============================================================
// Tcl Command: read_netlist
// Usage: read_netlist <filename>
// ============================================================
int ReadNetlistCmd(ClientData /*client_data*/, Tcl_Interp* interp,
                   int objc, Tcl_Obj* const objv[]) {
    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "<filename>");
        return TCL_ERROR;
    }

    std::string filename = GetArg(interp, objv, 1);
    auto result = g_parser.parse_file(filename, g_engine);

    if (!result.success) {
        std::string err = "Parse errors:\n";
        for (const auto& e : result.errors) {
            err += "  " + e + "\n";
        }
        Tcl_SetResult(interp, (char*)err.c_str(), TCL_VOLATILE);
        return TCL_ERROR;
    }

    // Print summary
    printf("Loaded netlist: %s\n", filename.c_str());
    printf("  Clocks: %d\n", result.clocks_created);
    printf("  Cells:  %d\n", result.cells_created);
    printf("  Nets:   %d\n", result.nets_created);
    printf("  Ports:  %d\n", result.ports_created);

    return TCL_OK;
}

// ============================================================
// Tcl Command: report_design
// Usage: report_design
// ============================================================
int ReportDesignCmd(ClientData /*client_data*/, Tcl_Interp* interp,
                    int objc, Tcl_Obj* const objv[]) {
    if (objc != 1) {
        Tcl_WrongNumArgs(interp, 1, objv, nullptr);
        return TCL_ERROR;
    }

    std::string report;
    report += "================================================================\n";
    report += "                     Design Summary\n";
    report += "================================================================\n\n";

    // Clocks
    auto clocks = g_engine.get_all_clocks();
    report += "Clocks (" + std::to_string(clocks.size()) + "):\n";
    for (const auto& name : clocks) {
        auto clk = g_engine.get_clock(name);
        if (clk) {
            report += "  " + name + ": period=" + std::to_string(clk->period) +
                      " skew=" + std::to_string(clk->skew) + "\n";
        }
    }
    report += "\n";

    // Cells
    auto cells = g_engine.get_all_cells();
    report += "Cells (" + std::to_string(cells.size()) + "):\n";
    for (const auto& name : cells) {
        auto cell = g_engine.get_cell(name);
        if (cell) {
            report += "  " + name + ": type=" + cell->type +
                      " tcq=" + std::to_string(cell->tcq) +
                      " tsetup=" + std::to_string(cell->tsetup) +
                      " thold=" + std::to_string(cell->thold) + "\n";
        }
    }
    report += "\n";

    // Nets
    auto nets = g_engine.get_all_nets();
    report += "Nets (" + std::to_string(nets.size()) + "):\n";
    for (const auto& name : nets) {
        auto net = g_engine.get_net(name);
        if (net) {
            report += "  " + name + ": " + net->from_cell + "/" + net->from_pin +
                      " -> " + net->to_cell + "/" + net->to_pin +
                      " delay=" + std::to_string(net->delay) + "\n";
        }
    }
    report += "\n";
    report += "================================================================\n";

    Tcl_SetObjResult(interp, Tcl_NewStringObj(report.c_str(), -1));
    return TCL_OK;
}

// ============================================================
// Tcl Command: check_timing
// Usage: check_timing [-setup_only] [-hold_only]
// ============================================================
int CheckTimingCmd(ClientData /*client_data*/, Tcl_Interp* interp,
                   int objc, Tcl_Obj* const objv[]) {
    bool setup_only = false;
    bool hold_only = false;

    for (int i = 1; i < objc; ++i) {
        std::string opt = GetArg(interp, objv, i);
        if (opt == "-setup_only") {
            setup_only = true;
        } else if (opt == "-hold_only") {
            hold_only = true;
        }
    }

    auto cells = g_engine.get_all_cells();
    int total = 0, pass = 0, fail = 0;

    std::string report;
    report += "================================================================\n";
    report += "                     Timing Check Summary\n";
    report += "================================================================\n\n";

    // Check all cell pairs (simplified: check sequential pairs)
    for (size_t i = 0; i < cells.size(); ++i) {
        for (size_t j = i + 1; j < cells.size(); ++j) {
            auto setup = g_engine.check_setup(cells[i], cells[j]);
            auto hold = g_engine.check_hold(cells[i], cells[j]);

            // Only count if there's a valid path
            if (setup.data_arrival > 0 || hold.data_arrival > 0) {
                ++total;
                if (setup.pass && hold.pass) {
                    ++pass;
                } else {
                    ++fail;
                    report += "VIOLATION: " + cells[i] + " -> " + cells[j] + "\n";
                    if (!setup.pass) {
                        report += "  Setup Slack: " + std::to_string(setup.slack) + " (FAIL)\n";
                    }
                    if (!hold.pass) {
                        report += "  Hold Slack: " + std::to_string(hold.slack) + " (FAIL)\n";
                    }
                }
            }
        }
    }

    report += "\nTotal paths: " + std::to_string(total) + "\n";
    report += "Pass: " + std::to_string(pass) + "\n";
    report += "Fail: " + std::to_string(fail) + "\n";
    report += "\n================================================================\n";

    Tcl_SetObjResult(interp, Tcl_NewStringObj(report.c_str(), -1));
    return TCL_OK;
}

// ============================================================
// Tcl Command: get_clocks / get_cells / get_nets
// Usage: get_clocks [pattern]
// ============================================================
int GetClocksCmd(ClientData /*client_data*/, Tcl_Interp* interp,
                 int objc, Tcl_Obj* const objv[]) {
    auto clocks = g_engine.get_all_clocks();
    std::string result;
    for (const auto& name : clocks) {
        result += name + " ";
    }
    Tcl_SetObjResult(interp, Tcl_NewStringObj(result.c_str(), -1));
    return TCL_OK;
}

int GetCellsCmd(ClientData /*client_data*/, Tcl_Interp* interp,
                int objc, Tcl_Obj* const objv[]) {
    auto cells = g_engine.get_all_cells();
    std::string result;
    for (const auto& name : cells) {
        result += name + " ";
    }
    Tcl_SetObjResult(interp, Tcl_NewStringObj(result.c_str(), -1));
    return TCL_OK;
}

int GetNetsCmd(ClientData /*client_data*/, Tcl_Interp* interp,
               int objc, Tcl_Obj* const objv[]) {
    auto nets = g_engine.get_all_nets();
    std::string result;
    for (const auto& name : nets) {
        result += name + " ";
    }
    Tcl_SetObjResult(interp, Tcl_NewStringObj(result.c_str(), -1));
    return TCL_OK;
}

// ============================================================
// Register All Commands
// ============================================================
void RegisterCommands(Tcl_Interp* interp) {
    // Demo commands
    Tcl_CreateObjCommand(interp, "hello", HelloCmd, nullptr, nullptr);
    Tcl_CreateObjCommand(interp, "minista_version", MinistaVersionCmd,
                         nullptr, nullptr);

    // Design commands
    Tcl_CreateObjCommand(interp, "create_clock", CreateClockCmd,
                         nullptr, nullptr);
    Tcl_CreateObjCommand(interp, "create_cell", CreateCellCmd,
                         nullptr, nullptr);
    Tcl_CreateObjCommand(interp, "create_net", CreateNetCmd,
                         nullptr, nullptr);

    // Analysis commands
    Tcl_CreateObjCommand(interp, "report_timing", ReportTimingCmd,
                         nullptr, nullptr);
    Tcl_CreateObjCommand(interp, "report_design", ReportDesignCmd,
                         nullptr, nullptr);
    Tcl_CreateObjCommand(interp, "check_timing", CheckTimingCmd,
                         nullptr, nullptr);

    // Query commands
    Tcl_CreateObjCommand(interp, "get_clocks", GetClocksCmd,
                         nullptr, nullptr);
    Tcl_CreateObjCommand(interp, "get_cells", GetCellsCmd,
                         nullptr, nullptr);
    Tcl_CreateObjCommand(interp, "get_nets", GetNetsCmd,
                         nullptr, nullptr);

    // I/O commands
    Tcl_CreateObjCommand(interp, "read_netlist", ReadNetlistCmd,
                         nullptr, nullptr);
}

// ============================================================
// Initialize Tcl Interpreter
// ============================================================
Tcl_Interp* CreateInterpreter() {
    Tcl_Interp* interp = Tcl_CreateInterp();
    if (interp == nullptr) {
        fprintf(stderr, "Error: Failed to create Tcl interpreter.\n");
        return nullptr;
    }

    if (Tcl_Init(interp) != TCL_OK) {
        fprintf(stderr, "Warning: Tcl_Init failed: %s\n",
                Tcl_GetStringResult(interp));
        Tcl_ResetResult(interp);
    }

    RegisterCommands(interp);
    return interp;
}

// ============================================================
// Read-Eval-Print Loop (REPL) with readline support
// ============================================================
void RunRepl(Tcl_Interp* interp) {
    printf("%s\n", minista::kProjectBanner);

    // Enable readline history
    using_history();

    while (true) {
        // readline provides line editing, history (up/down arrows)
        char* input = readline("tcl> ");

        // EOF (Ctrl+D)
        if (input == nullptr) {
            printf("\n");
            break;
        }

        // Skip empty lines
        if (input[0] == '\0') {
            free(input);
            continue;
        }

        // Check for exit/quit
        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            free(input);
            break;
        }

        // Add to history (so up-arrow recalls it)
        add_history(input);

        // Evaluate the Tcl command
        int result = Tcl_Eval(interp, input);

        if (result == TCL_OK) {
            const char* result_str = Tcl_GetStringResult(interp);
            if (result_str != nullptr && result_str[0] != '\0') {
                printf("%s\n", result_str);
            }
        } else {
            const char* error_info = Tcl_GetVar(interp, "errorInfo",
                                                TCL_GLOBAL_ONLY);
            if (error_info != nullptr && error_info[0] != '\0') {
                printf("Error: %s\n", error_info);
            } else {
                printf("Error: %s\n", Tcl_GetStringResult(interp));
            }
        }

        free(input);
    }
}

}  // namespace

// ============================================================
// Main
// ============================================================
int main(int /*argc*/, char* /*argv*/[]) {
    Tcl_Interp* interp = CreateInterpreter();
    if (interp == nullptr) {
        return EXIT_FAILURE;
    }

    RunRepl(interp);

    Tcl_DeleteInterp(interp);
    return EXIT_SUCCESS;
}
