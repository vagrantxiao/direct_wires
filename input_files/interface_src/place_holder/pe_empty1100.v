module pe_empty1100#(
    parameter EAST_WIDTH = 130,
    parameter WEST_WIDTH = 130,
    parameter NORTH_WIDTH = 130,
    parameter SOUTH_WIDTH = 130,
    parameter NUM_BRAM_ADDR_BITS=7,
    parameter DUMMY = 130
   )
    (
  input ap_start,
  input [WEST_WIDTH-1:0] in_from_west,
  input [NORTH_WIDTH-1:0] in_from_north,

  output reg [WEST_WIDTH-1:0] out_to_west,
  output reg [NORTH_WIDTH-1:0] out_to_north,

  input clk,
  input reset
  );

  always@(posedge clk) begin
    if(reset) begin
      out_to_west  <= 0;
      out_to_north <= 0;
    end else if(ap_start) begin
      out_to_west  <= in_from_west;
      out_to_north <= in_from_north;
    end else begin
      out_to_west  <= out_to_west;
      out_to_north <= out_to_north;
    end
  end


endmodule

