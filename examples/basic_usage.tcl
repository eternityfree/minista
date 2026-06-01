# basic_usage.tcl — Basic MiniSTA usage example.
#
# This script demonstrates the core workflow of MiniSTA:
#   1. Define clocks
#   2. Create sequential elements (registers)
#   3. Connect them with nets and combinational logic
#   4. Run timing analysis
#
# Run with: ./minista < examples/basic_usage.tcl

# ====================
# Step 1: Define Clock
# ====================
puts "=== Defining Clock ==="
create_clock -name main_clk -period 10.0

# ====================
# Step 2: Create Registers
# ====================
puts "=== Creating Registers ==="
create_cell -name u_reg1 -type DFF -tcq 0.5 -tsetup 0.2 -thold 0.1
create_cell -name u_reg2 -type DFF -tcq 0.5 -tsetup 0.2 -thold 0.1

# ====================
# Step 3: Define Data Paths
# ====================
puts "=== Defining Data Path ==="
create_net -name data_path -from u_reg1/Q -to u_reg2/D
set_comb_delay -net data_path -delay 3.0

# ====================
# Step 4: Run Timing Analysis
# ====================
puts "=== Timing Analysis ==="
report_timing -from u_reg1 -to u_reg2
report_timing -from u_reg1 -to u_reg2 -hold
report_timing -from u_reg1 -to u_reg2 -diagram

# ====================
# Step 5: Batch Check
# ====================
puts "=== Check All Paths ==="
check_timing
