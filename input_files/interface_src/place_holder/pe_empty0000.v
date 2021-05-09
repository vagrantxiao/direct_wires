module pe_empty0000#(
    parameter EAST_WIDTH = 130,
    parameter WEST_WIDTH = 130,
    parameter NORTH_WIDTH = 130,
    parameter SOUTH_WIDTH = 130,
    parameter NUM_BRAM_ADDR_BITS=7,
    parameter DUMMY = 130
   )
    (
  input ap_start,


  input clk,
  input reset
  );

  always@(posedge clk) begin
    if(reset) begin
    end else if(ap_start) begin
    end else begin
    end
  end


endmodule

