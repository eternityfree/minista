// Copyright 2026 MiniSTA Project
// Licensed under the MIT License.
//
// test_main.cpp — Unit tests for MiniSTA.
//
// This file contains basic unit tests using Google Test.
// It verifies the core type definitions and Tcl integration.

#include <gtest/gtest.h>
#include <tcl.h>

#include "minista/minista.h"
#include "minista/types.h"

namespace minista {
namespace {

// ============================================================
// Version Tests
// ============================================================

TEST(VersionTest, MajorVersionIsNonNegative) {
    EXPECT_GE(Version::kMajor, 0);
}

TEST(VersionTest, MinorVersionIsNonNegative) {
    EXPECT_GE(Version::kMinor, 0);
}

TEST(VersionTest, PatchVersionIsNonNegative) {
    EXPECT_GE(Version::kPatch, 0);
}

TEST(VersionTest, StringFormatIsCorrect) {
    const char* version = Version::String();
    ASSERT_NE(version, nullptr);
    // Version string should be "MAJOR.MINOR.PATCH"
    EXPECT_STREQ(version, "0.1.0");
}

// ============================================================
// Types Tests
// ============================================================

TEST(CellTypeTest, DffToString) {
    EXPECT_STREQ(CellTypeToString(CellType::kDff), "DFF");
}

TEST(CellTypeTest, LatchToString) {
    EXPECT_STREQ(CellTypeToString(CellType::kLatch), "LATCH");
}

TEST(CellTypeTest, UnknownToString) {
    EXPECT_STREQ(CellTypeToString(CellType::kUnknown), "UNKNOWN");
}

TEST(PathTypeTest, SetupToString) {
    EXPECT_STREQ(PathTypeToString(PathType::kSetup), "Setup");
}

TEST(PathTypeTest, HoldToString) {
    EXPECT_STREQ(PathTypeToString(PathType::kHold), "Hold");
}

TEST(TimingResultTest, PassToString) {
    EXPECT_STREQ(TimingResultToString(TimingResult::kPass), "PASS");
}

TEST(TimingResultTest, FailToString) {
    EXPECT_STREQ(TimingResultToString(TimingResult::kFail), "FAIL");
}

// ============================================================
// Tcl Integration Tests
// ============================================================

class TclInterpreterTest : public ::testing::Test {
protected:
    void SetUp() override {
        interp_ = Tcl_CreateInterp();
        ASSERT_NE(interp_, nullptr) << "Failed to create Tcl interpreter";
    }

    void TearDown() override {
        if (interp_ != nullptr) {
            Tcl_DeleteInterp(interp_);
            interp_ = nullptr;
        }
    }

    Tcl_Interp* interp_ = nullptr;
};

TEST_F(TclInterpreterTest, InterpreterCreation) {
    // Interpreter should be created successfully in SetUp
    EXPECT_NE(interp_, nullptr);
}

TEST_F(TclInterpreterTest, BasicTclEvaluation) {
    // Tcl should be able to evaluate basic expressions
    int result = Tcl_Eval(interp_, "expr {2 + 3}");
    EXPECT_EQ(result, TCL_OK);

    const char* result_str = Tcl_GetStringResult(interp_);
    ASSERT_NE(result_str, nullptr);
    EXPECT_STREQ(result_str, "5");
}

TEST_F(TclInterpreterTest, VariableAssignment) {
    // Tcl should support variable assignment and retrieval
    int result = Tcl_Eval(interp_, "set myvar 42");
    EXPECT_EQ(result, TCL_OK);

    const char* value = Tcl_GetVar(interp_, "myvar", TCL_GLOBAL_ONLY);
    ASSERT_NE(value, nullptr);
    EXPECT_STREQ(value, "42");
}

TEST_F(TclInterpreterTest, StringOperations) {
    // Tcl should handle string operations
    int result = Tcl_Eval(interp_, "string length \"hello\"");
    EXPECT_EQ(result, TCL_OK);

    const char* result_str = Tcl_GetStringResult(interp_);
    ASSERT_NE(result_str, nullptr);
    EXPECT_STREQ(result_str, "5");
}

TEST_F(TclInterpreterTest, ErrorHandling) {
    // Invalid commands should return TCL_ERROR
    int result = Tcl_Eval(interp_, "nonexistent_command_12345");
    EXPECT_EQ(result, TCL_ERROR);
}

// ============================================================
// Custom Command Test (hello)
// ============================================================

// Helper: register the hello command (same as in main.cpp)
static int TestHelloCmd(ClientData /*client_data*/, Tcl_Interp* interp,
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

TEST_F(TclInterpreterTest, HelloCommandDefault) {
    Tcl_CreateObjCommand(interp_, "hello", TestHelloCmd, nullptr, nullptr);

    int result = Tcl_Eval(interp_, "hello");
    EXPECT_EQ(result, TCL_OK);
    EXPECT_STREQ(Tcl_GetStringResult(interp_), "Hello, World!");
}

TEST_F(TclInterpreterTest, HelloCommandWithName) {
    Tcl_CreateObjCommand(interp_, "hello", TestHelloCmd, nullptr, nullptr);

    int result = Tcl_Eval(interp_, "hello MiniSTA");
    EXPECT_EQ(result, TCL_OK);
    EXPECT_STREQ(Tcl_GetStringResult(interp_), "Hello, MiniSTA!");
}

TEST_F(TclInterpreterTest, HelloCommandTooManyArgs) {
    Tcl_CreateObjCommand(interp_, "hello", TestHelloCmd, nullptr, nullptr);

    int result = Tcl_Eval(interp_, "hello a b");
    EXPECT_EQ(result, TCL_ERROR);
}

// ============================================================
// Project Constants Tests
// ============================================================

TEST(ProjectConstantsTest, NameIsSet) {
    EXPECT_STREQ(kProjectName, "MiniSTA");
}

TEST(ProjectConstantsTest, DescriptionIsSet) {
    EXPECT_NE(kProjectDescription, nullptr);
    EXPECT_GT(strlen(kProjectDescription), 0u);
}

TEST(ProjectConstantsTest, BannerContainsVersion) {
    EXPECT_NE(kProjectBanner, nullptr);
    // Banner should contain the version string
    EXPECT_NE(strstr(kProjectBanner, Version::String()), nullptr);
}

}  // namespace
}  // namespace minista
