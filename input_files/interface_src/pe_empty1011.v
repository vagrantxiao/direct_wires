module pe_empty1011#(
    parameter EAST_WIDTH = 130,
    parameter WEST_WIDTH = 130,
    parameter NORTH_WIDTH = 130,
    parameter SOUTH_WIDTH = 130,
    parameter NUM_BRAM_ADDR_BITS = 7,
    parameter DUMMY_WIDTH = 130

 
   )
    (
  input ap_start,
  input [EAST_WIDTH-1:0] in_from_east,
  input [WEST_WIDTH-1:0] in_from_west,
  input [SOUTH_WIDTH-1:0] in_from_south,

  output [EAST_WIDTH-1:0] out_to_east,
  output [WEST_WIDTH-1:0] out_to_west,
  output [SOUTH_WIDTH-1:0] out_to_south,

  input clk,
  input reset
  );


endmodule
