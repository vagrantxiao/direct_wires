`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company:
// Engineer:
//
// Create Date: 03/23/2018 02:46:07 PM
// Design Name:
// Module Name: leaf_empty
// Project Name:
// Target Devices:
// Tool Versions:
// Description:
//
// Dependencies:
//
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
//
//////////////////////////////////////////////////////////////////////////////////

module leaf_empty(
    input wire clk_bft,
    input wire clk_user,
    input wire [48 : 0] din_leaf_bft2interface,
    output wire [48 : 0] dout_leaf_interface2bft,
    input wire resend,
    input wire reset_bft,
    input wire reset
    );
/*
    wire [31:0] dout_leaf_interface2user;
    wire [31:0] din_leaf_user2interface;
    wire vld_interface2user;
    wire ack_user2interface;
    wire vld_user2interface;
    wire ack_interface2user;
    
    leaf_interface #(
        .PACKET_BITS(49),
        .PAYLOAD_BITS(32), 
        .NUM_LEAF_BITS(5),
        .NUM_PORT_BITS(4),
        .NUM_ADDR_BITS(7),
        .NUM_IN_PORTS(1), 
        .NUM_OUT_PORTS(1),
        .NUM_BRAM_ADDR_BITS(7),
        .FREESPACE_UPDATE_SIZE(64)
    )leaf_interface_inst(
        .clk_bft(clk_bft),
        .clk_user(clk_user),
        .reset(reset),
        .reset_bft(reset_bft),
        .din_leaf_bft2interface(din_leaf_bft2interface),
        .dout_leaf_interface2bft(dout_leaf_interface2bft),
        .resend(resend),
        .dout_leaf_interface2user(dout_leaf_interface2user),
        .vld_interface2user(vld_interface2user),
        .ack_user2interface(ack_user2interface),
        .ack_interface2user(ack_interface2user),
        .vld_user2interface(vld_user2interface),
        .din_leaf_user2interface(din_leaf_user2interface)
    );
    
    user_kernel user_kernel_inst(
        .ap_clk(clk_user),
        .ap_rst(reset),
        .ap_start(1'b1),
        .ap_done(),
        .ap_idle(),
        .ap_ready(),
        .Input_1_V_V(dout_leaf_interface2user),
        .Input_1_V_V_ap_vld(vld_interface2user),
        .Input_1_V_V_ap_ack(ack_user2interface),
        .Output_1_V_V(din_leaf_user2interface),
        .Output_1_V_V_ap_vld(vld_user2interface),
        .Output_1_V_V_ap_ack(ack_interface2user)
        );  

*/
    
endmodule
