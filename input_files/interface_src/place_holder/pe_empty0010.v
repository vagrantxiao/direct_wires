module pe_empty0010#(
    parameter EAST_WIDTH = 130,
    parameter WEST_WIDTH = 130,
    parameter NORTH_WIDTH = 130,
    parameter SOUTH_WIDTH = 130,
    parameter NUM_BRAM_ADDR_BITS=7,
    parameter DUMMY = 130
   )
    (
  input ap_start,
  input [SOUTH_WIDTH-1:0] in_from_south,

  output reg [SOUTH_WIDTH-1:0] out_to_south,

  input clk,
  input reset
  );

  always@(posedge clk) begin
    if(reset) begin
      out_to_south <= 0;
    end else if(ap_start) begin
      out_to_south <= in_from_south;
    end else begin
      out_to_south <= out_to_south;
    end
  end


endmodule

