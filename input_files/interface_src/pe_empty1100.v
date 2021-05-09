module pe_empty1100#(
    parameter EAST_WIDTH = 130,
    parameter WEST_WIDTH = 130,
    parameter NORTH_WIDTH = 130,
    parameter SOUTH_WIDTH = 130,
    parameter NUM_BRAM_ADDR_BITS = 7,
    parameter DUMMY_WIDTH = 130

   )
    (
  input ap_start,
  input [WEST_WIDTH-1:0] in_from_west,
  input [NORTH_WIDTH-1:0] in_from_north,

  output [WEST_WIDTH-1:0] out_to_west,
  output [NORTH_WIDTH-1:0] out_to_north,

  input clk,
  input reset
  );


endmodule
