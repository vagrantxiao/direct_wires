#!/usr/bin/env python

### config_gen: generate the config.c and config.h files for PrFlow
# Created by: Yuanlong Xiao (yuanlongxiao24@gmail.com)
#         at: University of Pennsylvania
#         in: 2018       
           
#
# The big picture:
#
# config_gen will generate files by using different parameters
#
# How it works (in a nutshell):
#
# Run command "python config_gen.py"
#
# [1] D. Park, Y. Xiao and A. DeHon, "Case for Acceleration of FPGA Compilation using Partial Reconfiguration", FPL2018
#


# This module contains classes needed by config_gen.py 
import xml.etree.ElementTree
import time


class gen_mb:
  def __init__(self, arch_name):
    self.arch_name = arch_name
    self.tcl_out = '../../tcl_out.tcl'
    self.root = xml.etree.ElementTree.parse(self.arch_name).getroot()
    self.specs = self.root.findall('spec')
    self.functions = self.root.findall('function')
    self.links = self.root.findall('link')
    file_out = open(self.tcl_out, 'w')
    file_out.close()
	


  def connect_IP(self):
    file_out = open(self.tcl_out, 'a')
    mb_num = 1
    for child in self.links:
      source_function = child.get('source').split('.')[0]
      source_port = str(int(child.get('source').split('.')[1])+1)
      destination_function = child.get('destination').split('.')[0]
      destination_port = str(int(child.get('destination').split('.')[1])+1)

      if source_function == 'Function1' or destination_function == 'Function1' or destination_function == 'Function22' or destination_function == 'Function25':
        pass
      else:
        file_out.write('\n')
        file_out.write('connect_bd_net [get_bd_pins '+source_function+'/Output_'+source_port+'_V_V] [get_bd_pins microblaze_'+str(mb_num)+'/S1_AXIS_TDATA]\n')
        file_out.write('connect_bd_net [get_bd_pins '+source_function+'/Output_'+source_port+'_V_V_ap_vld] [get_bd_pins microblaze_'+str(mb_num)+'/S1_AXIS_TVALID]\n')
        file_out.write('connect_bd_net [get_bd_pins microblaze_'+str(mb_num)+'/S1_AXIS_TREADY] [get_bd_pins '+source_function+'/Output_'+source_port+'_V_V_ap_ack]\n')
        file_out.write('connect_bd_net [get_bd_pins microblaze_'+str(mb_num)+'/M1_AXIS_TDATA] [get_bd_pins '+destination_function+'/Input_'+destination_port+'_V_V]\n')
        file_out.write('connect_bd_net [get_bd_pins microblaze_'+str(mb_num)+'/M1_AXIS_TVALID] [get_bd_pins '+destination_function+'/Input_'+destination_port+'_V_V_ap_vld]\n')
        file_out.write('connect_bd_net [get_bd_pins '+destination_function+'/Input_'+destination_port+'_V_V_ap_ack] [get_bd_pins microblaze_'+str(mb_num)+'/M1_AXIS_TREADY]\n')
        mb_num = mb_num+1
    file_out.close()



  def add_mb(self):
    file_out = open(self.tcl_out, 'a')
    for i in range(30):
      file_out.write('\n')
      file_out.write('startgroup\n')
      file_out.write('create_bd_cell -type ip -vlnv xilinx.com:ip:microblaze:10.0 microblaze_'+str(i)+'\n')
      file_out.write('set_property -dict [list CONFIG.C_FSL_LINKS {2} CONFIG.C_USE_EXTENDED_FSL_INSTR {1}] [get_bd_cells microblaze_'+str(i)+']\n')
      file_out.write('endgroup\n')
      file_out.write('apply_bd_automation -rule xilinx.com:bd_rule:microblaze -config {axi_intc {0} axi_periph {Disabled} cache {None} clk {/Clk (100 MHz)} debug_module {Debug Only} ecc {None} local_mem {8KB} preset {None}}  [get_bd_cells microblaze_'+str(i)+']\n')
      file_out.write('connect_bd_net [get_bd_ports ext_reset_in] [get_bd_pins rst_Clk_100M_'+str(i-1)+'/ext_reset_in]\n')
    file_out.close()




  def add_func_IP(self):
    file_out = open(self.tcl_out, 'a')
    for child in self.functions:
      file_out.write('\n')
      file_out.write('startgroup\n')
      file_out.write('create_bd_cell -type ip -vlnv xilinx.com:hls:'+child.get('name')+':1.0 '+child.get('page')+'\n')
      file_out.write('endgroup\n')
      file_out.write('connect_bd_net [get_bd_pins '+child.get('page')+'/ap_clk] [get_bd_pins zynq_ultra_ps_e_0/pl_clk0]\n')
      file_out.write('connect_bd_net [get_bd_pins '+child.get('page')+'/ap_rst] [get_bd_pins inv_0/Res]\n')
      file_out.write('connect_bd_net [get_bd_pins xlconstant_0/dout] [get_bd_pins '+child.get('page')+'/ap_start]\n')


    file_out.close()


  def create_topology(self):
    self.add_mb()
    self.add_func_IP()
    self.connect_IP()
  
  def connect_IP_IP(self):
    file_out = open(self.tcl_out, 'a')
    for child in self.links:
      source_function = child.get('source').split('.')[0]
      source_port = str(int(child.get('source').split('.')[1])+1)
      destination_function = child.get('destination').split('.')[0]
      destination_port = str(int(child.get('destination').split('.')[1])+1)

      if source_function == 'Function1' or destination_function == 'Function1':
        pass
      else:
        file_out.write('\n')
        file_out.write('connect_bd_net [get_bd_pins '+source_function+'/Output_'+source_port+'_V_V] [get_bd_pins '+destination_function+'/Input_'+destination_port+'_V_V]\n')
        file_out.write('connect_bd_net [get_bd_pins '+source_function+'/Output_'+source_port+'_V_V_ap_vld] [get_bd_pins '+destination_function+'/Input_'+destination_port+'_V_V_ap_vld]\n')
        file_out.write('connect_bd_net [get_bd_pins '+destination_function+'/Input_'+destination_port+'_V_V_ap_ack] [get_bd_pins '+source_function+'/Output_'+source_port+'_V_V_ap_ack]\n')
    file_out.close()




if __name__ == '__main__':
  filename = '../input_files/hls_src/face_detection/architecture.xml'
  gen_mb_inst = gen_mb(filename)
  gen_mb_inst.connect_IP_IP()






















