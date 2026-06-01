# simple_design.tcl — Test design for MiniSTA integration tests.
#
# This script creates a simple two-register design:
#
#   reg1/CK --> reg1/Q --> [comb logic] --> reg2/D --> reg2/CK
#
# Clock period: 10ns
# Tcq: 0.5ns, Tsetup: 0.2ns, Thold: 0.1ns
# Combinational delay: 3.0ns
#
# Expected results:
#   Setup slack = 10.0 - (0.5 + 3.0 + 0.2) = 6.3ns (PASS)
#   Hold slack  = (0.5 + 3.0) - 0.1         = 3.4ns (PASS)

# Define clock
create_clock -name clk -period 10.0

# Define registers
create_cell -name reg1 -type DFF -tcq 0.5 -tsetup 0.2 -thold 0.1
create_cell -name reg2 -type DFF -tcq 0.5 -tsetup 0.2 -thold 0.1

# Define data path
create_net -name data_path -from reg1/Q -to reg2/D
set_comb_delay -net data_path -delay 3.0
