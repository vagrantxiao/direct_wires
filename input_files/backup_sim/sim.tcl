ungroup_bd_cells [get_bd_cells axi_leaf]

startgroup
delete_bd_objs [get_bd_nets axi_dma_0_m_axis_mm2s_tvalid] [get_bd_intf_nets axi_dma_0_M_AXI_SG] [get_bd_intf_nets axi_dma_0_M_AXI_MM2S] [get_bd_intf_nets ip2DMA_0_m_axis_mm2s] [get_bd_intf_nets ps8_0_axi_periph_M01_AXI] [get_bd_nets axi_dma_0_m_axis_mm2s_tdata] [get_bd_nets astream_shell_in1_ready_upward] [get_bd_intf_nets ps8_0_axi_periph_M00_AXI] [get_bd_intf_nets axi_smc_M00_AXI] [get_bd_intf_nets axi_dma_0_M_AXI_S2MM] [get_bd_nets xlconstant_0_dout] [get_bd_cells axi_smc] [get_bd_cells axi_dma_0] [get_bd_cells xlconstant_1] [get_bd_cells AxiLite2Bft_v2_0_0] [get_bd_cells xlconstant_0]
delete_bd_objs [get_bd_intf_nets zynq_ultra_ps_e_0_M_AXI_HPM0_FPD] [get_bd_intf_nets zynq_ultra_ps_e_0_M_AXI_HPM1_FPD] [get_bd_cells ps8_0_axi_periph]
endgroup


delete_bd_objs [get_bd_cells zynq_ultra_ps_e_0]

set_property  ip_repo_paths  {/home/ylxiao/ws_183/F200524/pr_flow/workspace/F007_mono_bft_rendering/ip_repo /home/ylxiao/ws_183/F200524/render_HLS} [current_project]




create_bd_port -dir I -type clk clk0
connect_bd_net [get_bd_ports clk0] [get_bd_pins rst_axi/slowest_sync_clk]

create_bd_port -dir I -type clk clk1
connect_bd_net [get_bd_ports clk1] [get_bd_pins rst_net_2/slowest_sync_clk]



create_bd_port -dir I -type rst reset_n
connect_bd_net [get_bd_ports reset_n] [get_bd_pins rst_axi/ext_reset_in]

startgroup
create_bd_cell -type ip -vlnv xilinx.com:hls:data_gen:1.0 data_gen_0
endgroup


connect_bd_net [get_bd_ports clk0] [get_bd_pins data_gen_0/ap_clk]

connect_bd_net [get_bd_pins rst_axi/peripheral_reset] [get_bd_pins data_gen_0/ap_rst]


create_bd_port -dir I ap_start
connect_bd_net [get_bd_pins /data_gen_0/ap_start] [get_bd_ports ap_start]

connect_bd_net [get_bd_pins data_gen_0/Output_1_V_V] [get_bd_pins astream_shell_in1/din]
connect_bd_net [get_bd_pins data_gen_0/Output_1_V_V_ap_vld] [get_bd_pins astream_shell_in1/val_in]
connect_bd_net [get_bd_pins astream_shell_in1/ready_upward] [get_bd_pins data_gen_0/Output_1_V_V_ap_ack]


startgroup
create_bd_port -dir O -from 127 -to 0 m_axis_mm2s_tdata
connect_bd_net [get_bd_pins /ip2DMA_0/m_axis_mm2s_tdata] [get_bd_ports m_axis_mm2s_tdata]
endgroup
startgroup
create_bd_port -dir O -from 15 -to 0 m_axis_mm2s_tkeep
connect_bd_net [get_bd_pins /ip2DMA_0/m_axis_mm2s_tkeep] [get_bd_ports m_axis_mm2s_tkeep]
endgroup
startgroup
create_bd_port -dir O m_axis_mm2s_tlast
connect_bd_net [get_bd_pins /ip2DMA_0/m_axis_mm2s_tlast] [get_bd_ports m_axis_mm2s_tlast]
endgroup
startgroup
create_bd_port -dir O m_axis_mm2s_tvalid
connect_bd_net [get_bd_pins /ip2DMA_0/m_axis_mm2s_tvalid] [get_bd_ports m_axis_mm2s_tvalid]
endgroup
startgroup
create_bd_port -dir I m_axis_mm2s_tready
connect_bd_net [get_bd_pins /ip2DMA_0/m_axis_mm2s_tready] [get_bd_ports m_axis_mm2s_tready]
endgroup


















