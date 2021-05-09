module pe_empty1001#(
    parameter EAST_WIDTH = 130,
    parameter WEST_WIDTH = 130,
    parameter NORTH_WIDTH = 130,
    parameter SOUTH_WIDTH = 130,
    parameter NUM_BRAM_ADDR_BITS=7,
    parameter DUMMY = 130
   )
    (
  input ap_start,
  input [EAST_WIDTH-1:0] in_from_east,
  input [WEST_WIDTH-1:0] in_from_west,

  output reg [EAST_WIDTH-1:0] out_to_east,
  output reg [WEST_WIDTH-1:0] out_to_west,

  input clk,
  input reset
  );

  always@(posedge clk) begin
    if(reset) begin
      out_to_east  <= 0;
      out_to_west  <= 0;
    end else if(ap_start) begin
      out_to_east  <= in_from_east;
      out_to_west  <= in_from_west;
    end else begin
      out_to_east  <= out_to_east;
      out_to_west  <= out_to_west;
    end
  end


endmodule

