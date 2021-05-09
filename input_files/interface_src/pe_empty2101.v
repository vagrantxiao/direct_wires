module pe_empty2101#(
    parameter AXIS_WIDTH = 128,
 
    parameter EAST_WIDTH = 130,
    parameter WEST_WIDTH = 130,
    parameter NORTH_WIDTH = 130,
    parameter SOUTH_WIDTH = 130,
    parameter NUM_BRAM_ADDR_BITS = 7,
    parameter DUMMY_WIDTH = 130

    )
    (
  input ap_start,
  input [AXIS_WIDTH-1:0] din,
  input val_in,
  output ready_upward,

  output [AXIS_WIDTH-1:0] dout,
  output val_out,
  input ready_downward,

  input [EAST_WIDTH-1:0] in_from_east,
  input [NORTH_WIDTH-1:0] in_from_north,

  output [EAST_WIDTH-1:0] out_to_east,
  output [NORTH_WIDTH-1:0] out_to_northh,

  input clk,
  input reset
  );


endmodule
