module net0#(
parameter PAYLOAD_BITS0 = a,
parameter PORT_NUM_IN0 = b,
parameter PORT_NUM_OUT0 = c,
parameter PAYLOAD_BITS1 = 128,
parameter PORT_NUM_IN1 = 1,
parameter PORT_NUM_OUT1 = 1,
parameter PAYLOAD_BITS2 = 128,
parameter PORT_NUM_IN2 = 1,
parameter PORT_NUM_OUT2 = 1,
parameter PAYLOAD_BITS3 = 128,
parameter PORT_NUM_IN3 = 1,
parameter PORT_NUM_OUT3 = 1,
parameter PAYLOAD_BITS4 = 128,
parameter PORT_NUM_IN4 = 1,
parameter PORT_NUM_OUT4 = 1,
parameter PAYLOAD_BITS_NET0 = 128,
parameter PORT_NUM_NET0 = 1,
parameter WIDTH_DUMMY = 0
  )(
hell
you
are
every
thing
hell
you
are
every
thing
hell
you
are
every
thing
hell
you
are
every
thing
hell
you
are
every
thing
hell
you
are
every
thing
hell
you
are
every
thing
hell
you
are
every
thing
hell
you
are
every
thing
hell
you
are
every
thing
hell
you
are
every
thing
hell
you
are
every
thing
hell
you
are
every
thing
input clk,
input reset,
  


input wire [PAYLOAD_BITS0*PORT_NUM_IN0-1:0] din0,
input wire [PORT_NUM_IN0-1:0] val_in0,
output wire [PORT_NUM_IN0-1:0] ready_upward0,

output wire [PAYLOAD_BITS0*PORT_NUM_OUT0-1:0] dout0,
output wire [PORT_NUM_OUT0-1:0] val_out0,
input wire [PORT_NUM_OUT0-1:0] ready_downward0,

input wire [PAYLOAD_BITS1*PORT_NUM_IN1-1:0] din1,
input wire [PORT_NUM_IN1-1:0] val_in1,
output wire [PORT_NUM_IN1-1:0] ready_upward1,

output wire [PAYLOAD_BITS1*PORT_NUM_OUT1-1:0] dout1,
output wire [PORT_NUM_OUT1-1:0] val_out1,
input wire [PORT_NUM_OUT1-1:0] ready_downward1,

input wire [PAYLOAD_BITS2*PORT_NUM_IN2-1:0] din2,
input wire [PORT_NUM_IN2-1:0] val_in2,
output wire [PORT_NUM_IN2-1:0] ready_upward2,

output wire [PAYLOAD_BITS2*PORT_NUM_OUT2-1:0] dout2,
output wire [PORT_NUM_OUT2-1:0] val_out2,
input wire [PORT_NUM_OUT2-1:0] ready_downward2,

input wire [PAYLOAD_BITS3*PORT_NUM_IN3-1:0] din3,
input wire [PORT_NUM_IN3-1:0] val_in3,
output wire [PORT_NUM_IN3-1:0] ready_upward3,

output wire [PAYLOAD_BITS3*PORT_NUM_OUT3-1:0] dout3,
output wire [PORT_NUM_OUT3-1:0] val_out3,
input wire [PORT_NUM_OUT3-1:0] ready_downward3,

input wire [PAYLOAD_BITS4*PORT_NUM_IN4-1:0] din4,
input wire [PORT_NUM_IN4-1:0] val_in4,
output wire [PORT_NUM_IN4-1:0] ready_upward4,

output wire [PAYLOAD_BITS4*PORT_NUM_OUT4-1:0] dout4,
output wire [PORT_NUM_OUT4-1:0] val_out4,
input wire [PORT_NUM_OUT4-1:0] ready_downward4,




input  wire [PAYLOAD_BITS_NET0*PORT_NUM_NET0-1:0] net0_din,
input  wire net0_val_in,
output wire net0_ready_upward,

output wire [PAYLOAD_BITS_NET0*PORT_NUM_NET0-1:0] net0_dout,
output wire [PORT_NUM_NET0-1: 0] net0_val_out,
input  wire [PORT_NUM_NET0-1: 0] net0_ready_downward
  );


endmodule


