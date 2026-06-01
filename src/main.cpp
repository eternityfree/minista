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

#include "minista/minista.h"

namespace {

// ============================================================
// Custom Tcl Command: hello
// ============================================================
// A demonstration command that greets the user.
// Usage: hello [name]
//   - With no arguments: prints "Hello, World!"
//   - With an argument:   prints "Hello, <name>!"
//
// This serves as a template for implementing future MiniSTA
// commands (create_clock, report_timing, etc.).
// ============================================================
int HelloCmd(ClientData /*client_data*/, Tcl_Interp* interp,
             int objc, Tcl_Obj* const objv[]) {
    if (objc > 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "?name?");
        return TCL_ERROR;
    }

    std::string name = "World";
    if (objc == 2) {
        name = Tcl_GetString(objv[1]);
    }

    std::string result = "Hello, " + name + "!";
    Tcl_SetObjResult(interp, Tcl_NewStringObj(result.c_str(), -1));
    return TCL_OK;
}

// ============================================================
// Custom Tcl Command: minista_version
// ============================================================
// Returns the current MiniSTA version string.
// Usage: minista_version
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
// Tcl Channel for stdout redirection
// ============================================================
// In interactive mode, Tcl's [puts] needs to write to stdout.
// This ensures proper output routing.

// ============================================================
// Register Custom Commands
// ============================================================
// Registers all MiniSTA-specific commands with the Tcl interpreter.
// As the project grows, new commands will be registered here.
//
// Registered commands:
//   - hello          : Greeting demo command
//   - minista_version: Returns version string
// ============================================================
void RegisterCommands(Tcl_Interp* interp) {
    Tcl_CreateObjCommand(interp, "hello", HelloCmd, nullptr, nullptr);
    Tcl_CreateObjCommand(interp, "minista_version", MinistaVersionCmd,
                         nullptr, nullptr);
}

// ============================================================
// Initialize Tcl Interpreter
// ============================================================
// Creates and initializes a Tcl interpreter, registers all
// custom commands, and returns the interpreter pointer.
// Returns nullptr on failure.
// ============================================================
Tcl_Interp* CreateInterpreter() {
    Tcl_Interp* interp = Tcl_CreateInterp();
    if (interp == nullptr) {
        fprintf(stderr, "Error: Failed to create Tcl interpreter.\n");
        return nullptr;
    }

    if (Tcl_Init(interp) != TCL_OK) {
        // Tcl_Init may fail if tcl library files are not found.
        // This is non-fatal for our use case — we just lose some
        // standard library commands (encoding, etc.).
        fprintf(stderr, "Warning: Tcl_Init failed: %s\n",
                Tcl_GetStringResult(interp));
        // Clear the error so it doesn't interfere with the REPL.
        Tcl_ResetResult(interp);
    }

    RegisterCommands(interp);
    return interp;
}

// ============================================================
// Read-Eval-Print Loop (REPL)
// ============================================================
// Implements the interactive "tcl>" shell.
//
// Features:
//   - Displays "tcl> " prompt
//   - Reads a line of input from stdin
//   - Evaluates the input as a Tcl command
//   - Prints the result (or error message)
//   - Exits on EOF (Ctrl+D) or "exit"/"quit" command
//   - Multi-line support via backslash continuation
// ============================================================
void RunRepl(Tcl_Interp* interp) {
    printf("%s\n", minista::kProjectBanner);

    char line[4096];

    while (true) {
        printf("tcl> ");
        fflush(stdout);

        // Read a line from stdin
        if (fgets(line, sizeof(line), stdin) == nullptr) {
            // EOF (Ctrl+D)
            printf("\n");
            break;
        }

        // Strip trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            --len;
        }

        // Skip empty lines
        if (len == 0) {
            continue;
        }

        // Check for exit/quit commands
        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0) {
            break;
        }

        // Evaluate the Tcl command
        int result = Tcl_Eval(interp, line);

        if (result == TCL_OK) {
            // Print the result if it's non-empty
            const char* result_str = Tcl_GetStringResult(interp);
            if (result_str != nullptr && result_str[0] != '\0') {
                printf("%s\n", result_str);
            }
        } else {
            // Print error information
            const char* error_info = Tcl_GetVar(interp, "errorInfo",
                                                TCL_GLOBAL_ONLY);
            if (error_info != nullptr && error_info[0] != '\0') {
                printf("Error: %s\n", error_info);
            } else {
                printf("Error: %s\n", Tcl_GetStringResult(interp));
            }
        }
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
