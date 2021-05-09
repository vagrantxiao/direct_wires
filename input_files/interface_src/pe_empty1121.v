module pe_empty1121#(
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

  input [WEST_WIDTH-1:0] in_from_west,
  input [EAST_WIDTH-1:0] in_from_east,
  input [NORTH_WIDTH-1:0] in_from_north,

  output [WEST_WIDTH-1:0] out_to_west,
  output [EAST_WIDTH-1:0] out_to_east,
  output [NORTH_WIDTH-1:0] out_to_north,

  input clk,
  input reset
  );
/*
  always@(posedge clk) begin
    if(reset) begin
      dout <=0;
      val_out <= 0;
      ready_upward <= 0;
      out_to_east  <= 0;
      out_to_northh <= 0;
    end else if(ap_start) begin
      dout <=din;
      val_out <= val_in;
      ready_upward <= ready_downward;
      out_to_west  <= in_from_west;
      out_to_east  <= in_from_east;
      out_to_northh <= in_from_north;
    end else begin
      dout <=dout;
      val_out <= val_out;
      ready_upward <= ready_upward;
      out_to_west  <= out_to_west;
      out_to_east  <= out_to_east;
      out_to_northh <= out_to_northh;
    end
  end

*/

endmodule
