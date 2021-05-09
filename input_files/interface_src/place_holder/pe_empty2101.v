module pe_empty2101#(
    parameter AXIS_WIDTH = 128,
    parameter EAST_WIDTH = 130,
    parameter WEST_WIDTH = 130,
    parameter NORTH_WIDTH = 130,
    parameter NUM_BRAM_ADDR_BITS=7,
    parameter SOUTH_WIDTH = 130
    )
    (
  input ap_start,
  input [AXIS_WIDTH-1:0] din,
  input val_in,
  output reg ready_upward,

  output reg [AXIS_WIDTH-1:0] dout,
  output reg val_out,
  input ready_downward,

  input [EAST_WIDTH-1:0] in_from_east,
  input [NORTH_WIDTH-1:0] in_from_north,

  output reg [EAST_WIDTH-1:0] out_to_east,
  output reg [NORTH_WIDTH-1:0] out_to_northh,

  input clk,
  input reset
  );

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
      out_to_east  <= in_from_east;
      out_to_northh <= in_from_north;
    end else begin
      dout <=dout;
      val_out <= val_out;
      ready_upward <= ready_upward;
      out_to_east  <= out_to_east;
      out_to_northh <= out_to_northh;
    end
  end



endmodule
