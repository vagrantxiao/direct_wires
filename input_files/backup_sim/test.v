//Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2018.3 (lin64) Build 2405991 Thu Dec  6 23:36:41 MST 2018
//Date        : Sun May 24 16:48:56 2020
//Host        : ylxiao-OptiPlex-7050 running 64-bit Ubuntu 18.04.4 LTS
//Command     : generate_target floorplan_static_wrapper.bd
//Design      : floorplan_static_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ns / 1 ps

module test();

  reg ap_start;
  reg clk0;
  reg clk1;
  wire [127:0]m_axis_mm2s_tdata;
  wire [15:0]m_axis_mm2s_tkeep;
  wire m_axis_mm2s_tlast;
  reg m_axis_mm2s_tready;
  wire m_axis_mm2s_tvalid;
  reg reset_n;

floorplan_static_wrapper i1(
  .ap_start(ap_start),
  .clk0(clk0),
  .clk1(clk1),
  .m_axis_mm2s_tdata(m_axis_mm2s_tdata),
  .m_axis_mm2s_tkeep(m_axis_mm2s_tkeep),
  .m_axis_mm2s_tlast(m_axis_mm2s_tlast),
  .m_axis_mm2s_tready(m_axis_mm2s_tready),
  .m_axis_mm2s_tvalid(m_axis_mm2s_tvalid),
  .reset_n(reset_n));

always # 1 clk0=~clk0;
always # 1 clk1=~clk1;

initial begin
  clk0=0;
  clk1=1;
  reset_n=0;
  m_axis_mm2s_tready=0;
  ap_start = 0;
  #1007
  reset_n=1;
  #1000
  ap_start = 1;
  #10
  ap_start = 0;
  m_axis_mm2s_tready=1;
  #1000_000_000
  $stop();
end




endmodule

