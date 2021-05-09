import os  
import subprocess
from gen_basic import gen_basic

class gen_mono_bft(gen_basic):

  def return_direction_num(self, direction):
    direction_dict = {'W': 0, 'N': 1, 'S': 2, 'E': 3}
    return direction_dict[direction]
  
 
  # modify the project_syn2gen
  # replace the empty pages with real pages
  def return_syn2gen_tcl_list_local(self):
    # map of reset source for each pages
    # distributed resetting is good for timing
    mapping_dict = self.prflow_params['mapping_dict']
    datawidth_dict = self.prflow_params['datawidth_dict'] 
    links = self.prflow_params['links']
    self.used_pages = self.get_all_used_pages(links)
    place_holder_dict = self.prflow_params['place_holder_dict']

    lines_list = []                    
    lines_list.append('set_property  ip_repo_paths  {./ip_repo} [current_project]\n')
    lines_list.append('update_ip_catalog\n')

    for i in range(2,4):
      for j in range(3):
        lines_list.append(self.return_change_parameter_tcl_str('pe_empty_X'+str(i)+'Y'+str(j), 'WEST_WIDTH',  datawidth_dict['X'+str(i)+'Y'+str(j)][0]))
        lines_list.append(self.return_change_parameter_tcl_str('pe_empty_X'+str(i)+'Y'+str(j), 'NORTH_WIDTH', datawidth_dict['X'+str(i)+'Y'+str(j)][1]))
        lines_list.append(self.return_change_parameter_tcl_str('pe_empty_X'+str(i)+'Y'+str(j), 'SOUTH_WIDTH', datawidth_dict['X'+str(i)+'Y'+str(j)][2]))
        lines_list.append(self.return_change_parameter_tcl_str('pe_empty_X'+str(i)+'Y'+str(j), 'EAST_WIDTH',  datawidth_dict['X'+str(i)+'Y'+str(j)][3]))
    for i in range(4):
      for j in range(3,7):
        lines_list.append(self.return_change_parameter_tcl_str('pe_empty_X'+str(i)+'Y'+str(j), 'WEST_WIDTH',  datawidth_dict['X'+str(i)+'Y'+str(j)][0]))
        lines_list.append(self.return_change_parameter_tcl_str('pe_empty_X'+str(i)+'Y'+str(j), 'NORTH_WIDTH', datawidth_dict['X'+str(i)+'Y'+str(j)][1]))
        lines_list.append(self.return_change_parameter_tcl_str('pe_empty_X'+str(i)+'Y'+str(j), 'SOUTH_WIDTH', datawidth_dict['X'+str(i)+'Y'+str(j)][2]))
        lines_list.append(self.return_change_parameter_tcl_str('pe_empty_X'+str(i)+'Y'+str(j), 'EAST_WIDTH',  datawidth_dict['X'+str(i)+'Y'+str(j)][3]))
 


    # remove empty ip, add real IP and connect the wires
    for page in self.used_pages:
      page_num = page
      fun_name = self.get_key(page, mapping_dict)
      fun_num = self.get_entry(fun_name,  self.prflow_params['syn_fun_list'])
      dir_code = list(place_holder_dict[page_num].replace('pe_empty',''))
      X, Y =list(page_num.replace('X','').replace('Y',''))
      X = int(X)
      Y = int(Y)
      lines_list.append(self.return_delete_bd_objs_tcl_str('pe_empty_' + page_num))
      lines_list.append(self.return_create_bd_cell_tcl_str('user.org:user:pe_empty_' + page_num + ':1.0', 'pe_empty_' + page_num ))
      lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_'+page_num+'/ap_start', 'axi_leaf/ap_start'))       
      lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_'+page_num+'/clk', '/zynq_ultra_ps_e_0/pl_clk0'))
      lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_'+page_num+'/reset', 'rst_ps8_0_99M/peripheral_reset'))
      if dir_code[0] =='1': lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_' + page_num + '/in_from_west', 'pe_empty_X'+str(X-1)+'Y'+str(Y) + '/out_to_east'))
      if dir_code[1] =='1': lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_' + page_num + '/in_from_north', 'pe_empty_X'+str(X)+'Y'+str(Y+1) + '/out_to_south'))
      if dir_code[2] =='1': lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_' + page_num + '/in_from_south', 'pe_empty_X'+str(X)+'Y'+str(Y-1) + '/out_to_north'))
      if dir_code[3] =='1': lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_' + page_num + '/in_from_east', 'pe_empty_X'+str(X+1)+'Y'+str(Y) + '/out_to_west'))
      if dir_code[0] =='1': lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_' + page_num + '/out_to_west', 'pe_empty_X'+str(X-1)+'Y'+str(Y) + '/in_from_east'))
      if dir_code[1] =='1': lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_' + page_num + '/out_to_north', 'pe_empty_X'+str(X)+'Y'+str(Y+1) + '/in_from_south'))
      if dir_code[2] =='1': lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_' + page_num + '/out_to_south', 'pe_empty_X'+str(X)+'Y'+str(Y-1) + '/in_from_north'))
      if dir_code[3] =='1': lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_' + page_num + '/out_to_east', 'pe_empty_X'+str(X+1)+'Y'+str(Y) + '/in_from_west'))
      
      if dir_code[2] =='2': 
        lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_' + page_num + '/ready_downward', 'axi_leaf/ready'))
        lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_' + page_num + '/din', 'axi_leaf/m_axis_mm2s_tdata'))
        lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_' + page_num + '/val_in', 'axi_leaf/m_axis_mm2s_tvalid'))
        lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_' + page_num + '/dout', 'axi_leaf/dout'))
        lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_' + page_num + '/val_out', 'axi_leaf/valid'))
        lines_list.append(self.return_connect_bd_net_tcl_str('pe_empty_' + page_num + '/ready_upward', 'axi_leaf/m_axis_mm2s_tready'))


    return lines_list

  # main.sh will be used for local compilation
  def return_main_sh_list_local(self):
    lines_list = []
    lines_list.append('#!/bin/bash -e')
    lines_list.append('source '+self.prflow_params['Xilinx_dir'])
    # compile the IP for each page
    for fun_name in self.prflow_params['syn_fun_list']:
      if (fun_name !='user_kernel'): 
        lines_list.append('cd ./ip_repo' + fun_name) 
        lines_list.append('./run.sh') 
        lines_list.append('cd -') 
    lines_list.append('vivado -mode batch -source project_syn2gen.tcl')
    lines_list.append('vivado -mode batch -source project_syn2bits.tcl')
    return lines_list

  # qsub_main.sh will be used for qsub compilation  
  def return_qsub_main_sh_list_local(self):
    # go through all the files and qsub the ip compilation tasks
    lines_list = self.return_qsub_scan_sh_list('./ip_repo')
    hold_jid = '$file_list'
   
    # after the ip compilation is done, we can construct the vivado momo bft project
    lines_list.append(self.return_qsub_command_str('./qsub_project_syn2gen.sh', hold_jid, 'bft_mono_syn2gen'))

    # we can accelerate the synthesis by compile each out-of-context modules in parallel
    lines_list.append(self.return_qsub_command_str('./qsub_sub_syn.sh', 'bft_mono_syn2gen', 'bft_mono_sub_syn'))
    return lines_list 

  # qsub_sub_syn.sh will go through all the out-of-context module directories and qsub each 
  # task by executing thn runme.sh, which is generated by vivado 
  def return_sub_syn_sh_list_local(self):
    lines_list = self.return_qsub_scan_sh_list('./prj/floorplan_static.runs', 'runme.sh')
    hold_jid = '$file_list'
    
    # after all the out-of-context compilations are done, we reopen the project and compile it to bits.
    lines_list.append(self.return_qsub_command_str('./qsub_project_syn2bits.sh', hold_jid, 'bft_mono_syn2bits'))

    # after the vivado project is implemented to bitstreams, we make an SDK project
    lines_list.append(self.return_qsub_command_str('./qsub_project_xsdk.sh', 'bft_mono_syn2bits', 'bft_mono_sdk'))

    return lines_list 

  def get_key(self, val, my_dict): 
    for key, value in my_dict.items(): 
      if val == value: 
        return key
    return 'switchbox'

  def get_entry(self, val, my_list):
    for i in range(len(my_list)): 
      if val == my_list[i]:
        return i 



  def return_RelayStation_inst_v_list(self, name, width, val_in, ready_upward, din, val_out, ready_downward, dout): 
    local_list = []
    local_list.append('  RelayStation #(')
    local_list.append('    .PAYLOAD_BITS('+str(width)+')')
    local_list.append('  )RS_'+name+'(')
    local_list.append('    .clk(clk),')
    local_list.append('    .din('+din+'),')
    local_list.append('    .val_in('+val_in+'),')
    local_list.append('    .ready_upward('+ready_upward+'),')
    local_list.append('    .dout('+dout+'),')
    local_list.append('    .val_out('+val_out+'),')
    local_list.append('    .ready_downward('+ready_downward+'),')
    local_list.append('    .reset(reset)); ')
    local_list.append('')
    
    return local_list
 
  def return_StreamOut_inst_v_list(self, name, width, val_in, ready_upward, din, val_out, ready_downward, dout):
    local_list = []
    local_list.append('wire ['+str(width)+'-1:0] '+din+';')
    local_list.append('wire '+val_in+';')
    local_list.append('wire '+ready_upward+';')
    local_list.append('stream_shell #(')
    local_list.append('    .PAYLOAD_BITS('+str(width)+'),')
    local_list.append('    .NUM_BRAM_ADDR_BITS(NUM_BRAM_ADDR_BITS)')
    local_list.append(') stream_out_'+name+'(')
    local_list.append('    .clk(clk),')
    local_list.append('    .reset(reset),')
    local_list.append('    .din('+din+'),')
    local_list.append('    .val_in('+val_in+'),')
    local_list.append('    .ready_upward('+ready_upward+'),')
    local_list.append('    .dout('+dout+'),')
    local_list.append('    .val_out('+val_out+'),')
    local_list.append('    .ready_downward('+ready_downward+'));')
    local_list.append('')

    return local_list

  def return_StreamIn_inst_v_list(self, name, width, val_in, ready_upward, din, val_out, ready_downward, dout):
    local_list = []
    local_list.append('wire ['+str(width)+'-1:0] '+dout+';')
    local_list.append('wire '+val_out+';')
    local_list.append('wire '+ready_downward+';')
    local_list.append('stream_shell #(')
    local_list.append('    .PAYLOAD_BITS('+str(width)+'),')
    local_list.append('    .NUM_BRAM_ADDR_BITS(NUM_BRAM_ADDR_BITS)')
    local_list.append(') stream_in_'+name+'(')
    local_list.append('    .clk(clk),')
    local_list.append('    .reset(reset),')
    local_list.append('    .din('+din+'),')
    local_list.append('    .val_in('+val_in+'),')
    local_list.append('    .ready_upward('+ready_upward+'),')
    local_list.append('    .dout('+dout+'),')
    local_list.append('    .val_out('+val_out+'),')
    local_list.append('    .ready_downward('+ready_downward+'));')
    local_list.append('')

    return local_list

  def return_Out_inst_v_list(self, width, val_in, ready_upward, din, val_out, ready_downward, dout):
    local_list = []
    local_list.append('wire ['+str(width)+'-1:0] '+din+';')
    local_list.append('wire '+val_in+';')
    local_list.append('wire '+ready_upward+';')
    local_list.append('')
    local_list.append('assign '+dout+' = '+din+';')
    local_list.append('assign '+val_out+' = ' + val_in+';')
    local_list.append('assign '+ready_upward+' = '+ready_downward+';')
    local_list.append('')

    return local_list



  #def return_In_inst_v_list(self, width, val_in, ready_upward, din, val_out, ready_downward, dout):
  #  local_list = []
  #  local_list.append('wire ['+str(width)+'-1:0] '+dout+';')
  #  local_list.append('wire '+val_out+';')
  #  local_list.append('wire '+ready_downward+';')
  #  local_list.append('')
  #  local_list.append('assign '+dout+' = '+din+';')
  #  local_list.append('assign '+val_out+' = ' + val_in+';')
  #  local_list.append('assign '+ready_upward+' = '+ready_downward+';')
  #  local_list.append('')

  #  return local_list


  def return_function_inst_v_list(self, fun_name, in_num, out_num):
    local_list = []
    local_list.append('    '+fun_name+' '+fun_name+'_inst(')
    local_list.append('        .ap_clk(clk),')
    local_list.append('        .ap_rst(reset),')
    local_list.append('        .ap_start(ap_start),')
    local_list.append('        .ap_done(),')
    local_list.append('        .ap_idle(ap_idle),')
    for i in range(int(in_num)):
      local_list.append('        .Input_'+str(i+1)+'_V_V(Input_'+str(i+1)+'_V_V),')
      local_list.append('        .Input_'+str(i+1)+'_V_V_ap_vld(Input_'+str(i+1)+'_V_V_ap_vld),')
      local_list.append('        .Input_'+str(i+1)+'_V_V_ap_ack(Input_'+str(i+1)+'_V_V_ap_ack),')
    for i in range(int(out_num)):
      local_list.append('        .Output_'+str(i+1)+'_V_V(Output_'+str(i+1)+'_V_V),')
      local_list.append('        .Output_'+str(i+1)+'_V_V_ap_vld(Output_'+str(i+1)+'_V_V_ap_vld),')
      local_list.append('        .Output_'+str(i+1)+'_V_V_ap_ack(Output_'+str(i+1)+'_V_V_ap_ack),')
    local_list.append('        .ap_ready(ap_ready)')
    local_list.append(');')
    local_list.append('')

    return local_list

  def return_assignment_v_list(self, inst_name, max_width, used_width):
    pattern = inst_name.replace('pe_empty','')
    pattern_list = list(pattern)
    local_list = []
    if max_width[0] > used_width[0]:
      if pattern_list[0] == '1': local_list.append('assign out_to_west['+str(int(max_width[0])-1)+':'+str(used_width[0])+'] = 0;')
    if max_width[1] > used_width[1]:
      if pattern_list[1] == '1': local_list.append('assign out_to_north['+str(int(max_width[1])-1)+':'+str(used_width[1])+'] = 0;')
    if max_width[2] > used_width[2]:
      if pattern_list[2] == '1': local_list.append('assign out_to_south['+str(int(max_width[2])-1)+':'+str(used_width[2])+'] = 0;')
    if max_width[3] > used_width[3]:
      if pattern_list[3] == '1': local_list.append('assign out_to_east['+str(int(max_width[3])-1)+':'+str(used_width[3])+'] = 0;')

    return local_list






  def modify_pages(self, links_dict, mapping_dict):
    for page in self.used_pages:
      fun_name = self.get_key(page, mapping_dict)
      local_link_dict = {}
      for value in links_dict[page]:
        if not (value[0] in local_link_dict.keys()):
          local_link_dict[value[0]] = [value]
        else:
          local_link_dict[value[0]].append(value) 

 
      RelayStation_v_list = [] 
      Stream_v_list = [] 

      for key_local, value_local in local_link_dict.items():

        if len(value_local) == 2:
        # This is a pass through link
          if value_local[0][4].split('->')[0] == fun_name:
            RelayStation.appned(return_RelayStation_inst_v_list(key_local, value_local[0][2], value_local[1][4],  value_local[1][5], value_local[1][6], value_local[0][4], value_local[0][5], value_local[0][6]))
          else:
            RelayStation_v_list.append(self.return_RelayStation_inst_v_list(key_local, value_local[0][2], value_local[0][5], value_local[0][6], value_local[0][7], value_local[1][5], value_local[1][6], value_local[1][7]))
        elif value_local[0][4].split('->')[0].split('.')[0] == fun_name:
        # This page is the source page
          port_num = int(value_local[0][4].split('->')[0].split('.')[1])
          Stream_v_list.append(self.return_Out_inst_v_list(
                                                                 value_local[0][2], 
                                                                 'Output_'+str(port_num+1)+'_V_V_ap_vld', 
                                                                 'Output_'+str(port_num+1)+'_V_V_ap_ack', 
                                                                 'Output_'+str(port_num+1)+'_V_V', 
                                                                 value_local[0][5], 
                                                                 value_local[0][6], 
                                                                 value_local[0][7]
                                                                )
                              )
        else :
        # This page is the destination page
          port_num = int(value_local[0][4].split('->')[1].split('.')[1])
          Stream_v_list.append(self.return_StreamIn_inst_v_list(
                                                                key_local,
                                                                value_local[0][2], 
                                                                value_local[0][5], 
                                                                value_local[0][6], 
                                                                value_local[0][7], 
                                                                'Input_'+str(port_num+1)+'_V_V_ap_vld',  
                                                                'Input_'+str(port_num+1)+'_V_V_ap_ack',
                                                                'Input_'+str(port_num+1)+'_V_V'
                                                               )
                              )

   
      for lines_list in RelayStation_v_list:
        self.add_lines(self.mono_bft_dir+'/ip_repo/'+page+'/pe_empty_'+page+'.v', 'endmodule',  lines_list) 
      for lines_list in Stream_v_list:
        if fun_name != 'DMA':
          self.add_lines(self.mono_bft_dir+'/ip_repo/'+page+'/pe_empty_'+page+'.v', 'endmodule',  lines_list) 

  def return_IO_edge_str(self, src_page, dst_page):
  
    X_src = int(src_page.split('X')[1].split('Y')[0])
    Y_src = int(src_page.split('X')[1].split('Y')[1])
    X_dst = int(dst_page.split('X')[1].split('Y')[0])
    Y_dst = int(dst_page.split('X')[1].split('Y')[1])
    if X_src == X_dst:
      if Y_src == Y_dst+1:
        return 'S',  'N' 
      elif Y_src == Y_dst-1:
        return 'N',  'S'
      else:
        return 'Error', 'Error'
    elif Y_src == Y_dst:
      if X_src == X_dst+1:
        return 'W',  'E' 
      elif X_src == X_dst-1:
        return 'E',  'W'
      else:
        return 'Error', 'Error'
    else:
      return 'Error', 'Error'
  
  
  # return out_edge, in_edge
  def return_IO_edge(self, src_page, dst_page):
  
    X_src = int(src_page.split('X')[1].split('Y')[0])
    Y_src = int(src_page.split('X')[1].split('Y')[1])
    X_dst = int(dst_page.split('X')[1].split('Y')[0])
    Y_dst = int(dst_page.split('X')[1].split('Y')[1])
    if X_src == X_dst:
      if Y_src == Y_dst+1:
        return self.return_direction_num('S'),  self.return_direction_num('N') 
      elif Y_src == Y_dst-1:
        return self.return_direction_num('N'),  self.return_direction_num('S')
      else:
        return 'Error', 'Error'
    elif Y_src == Y_dst:
      if X_src == X_dst+1:
        return self.return_direction_num('W'),  self.return_direction_num('E') 
      elif X_src == X_dst-1:
        return self.return_direction_num('E'),  self.return_direction_num('W')
      else:
        return 'Error', 'Error'
    else:
      return 'Error', 'Error'
 
  def parse_out_dir(self, out_dir):
    if out_dir == 'E':
      return 'out_to_east' 
    elif out_dir == 'W':
      return 'out_to_west' 
    elif out_dir == 'N':
      return 'out_to_north' 
    elif out_dir == 'S':
      return 'out_to_south' 

  def parse_in_dir(self, in_dir):
    if in_dir == 'E':
      return 'in_from_east' 
    elif in_dir == 'W':
      return 'in_from_west' 
    elif in_dir == 'N':
      return 'in_from_north' 
    elif in_dir == 'S':
      return 'in_from_south' 
 


  def return_page(self, str_in, mapping_dict):
    if str_in.startswith('X'):
      return str_in
    else:
      return mapping_dict[str_in]


  def gen_routing_dict(self, links, mapping_dict):
    width_in_dict = {}
    width_out_dict = {}
    self.width_max_dict = {}
    links_dict = {}
    for i in range(4):
      for j in range(7):
        width_in_dict['X'+str(i)+'Y'+str(j)] = [0,0,0,0]
        width_out_dict['X'+str(i)+'Y'+str(j)] = [0,0,0,0]
 

    link_num = 0; 
    for child in links:
      link_num+=1
      link_chain = child.get('source').split('-')
      for i in range(len(link_chain)-1):
        src_str_list = []
        dst_str_list = []
        src_page  = self.return_page(link_chain[i].split('.')[0], mapping_dict)
        dest_page = self.return_page(link_chain[i+1].split('.')[0], mapping_dict)
        out_dir_num, in_dir_num = self.return_IO_edge(src_page, dest_page)
        out_dir_str, in_dir_str = self.return_IO_edge_str(src_page, dest_page)
        width = int(child.get('width'))


        # For the output assignments
        if not (src_page in links_dict.keys()): links_dict[src_page] = []
        links_dict[src_page].append([
                                     'link'+str(link_num),
                                     'source',
                                     str(width),
                                     out_dir_str,
                                     link_chain[i]+'->'+link_chain[i+1], 
                                     self.parse_out_dir(out_dir_str)+'['+str(width_out_dict[src_page][out_dir_num])+']' if link_chain[i+1].split('.')[0] != 'DMA' else 'val_out', 
                                     self.parse_in_dir(out_dir_str)+'['+str(width_in_dict[src_page][out_dir_num])+']' if link_chain[i+1].split('.')[0] != 'DMA' else 'ready_downward', 
                                     self.parse_out_dir(out_dir_str)+'['+str(width_out_dict[src_page][out_dir_num]+width)+':'+str(width_out_dict[src_page][out_dir_num]+1)+']' if link_chain[i+1].split('.')[0] != 'DMA' else 'dout' 
                                     ]) 

        width_out_dict[src_page][out_dir_num]+=(width+1)  
        width_in_dict[src_page][out_dir_num]+=1

        # For the input assignments
        if not (dest_page in links_dict.keys()): links_dict[dest_page] = []
        links_dict[dest_page].append([
                                     'link'+str(link_num),
                                      'sink', 
                                      str(width),
                                      in_dir_str,
                                      link_chain[i]+'->'+link_chain[i+1], 
                                      self.parse_in_dir(in_dir_str)+'['+str(width_in_dict[dest_page][in_dir_num])+']'if link_chain[i].split('.')[0] != 'DMA' else 'val_in', 
                                      self.parse_out_dir(in_dir_str)+'['+str(width_out_dict[dest_page][in_dir_num])+']'if link_chain[i].split('.')[0] != 'DMA' else 'ready_upward', 
                                      self.parse_in_dir(in_dir_str)+'['+str(width_in_dict[dest_page][in_dir_num]+width)+':'+str(width_in_dict[dest_page][in_dir_num]+1)+']' if link_chain[i].split('.')[0] != 'DMA' else 'din'
                                     ])
 
        width_in_dict[dest_page][in_dir_num]+=(width+1)  
        width_out_dict[dest_page][in_dir_num]+=1

         

    #print "links_dict" 
    #for key in links_dict.keys():
    #  print key+'->'
    #  self.print_list(links_dict[key]) 
    #print ""


    for key in width_in_dict: 
      #value_in = width_in_dict[key]
      value_out = width_out_dict[key]
      #for i in range(len(value_in)):
      #  if value_in[i] < value_out[i]:
      #    value_in[i] = value_out[i]
      self.width_max_dict[key] = value_out
       
     
    return links_dict




 
  def return_run_sdk_sh_list_local(self, vivado_dir, tcl_file):
    return ([
      '#!/bin/bash -e',
      'source ' + vivado_dir,
      'xsdk -batch -source ' + tcl_file,
      ''])

  def get_all_used_pages(self, links):
    mapping_dict = self.prflow_params['mapping_dict']
    link_num = 0
    dummy_dict = {}
    for child in links:
      link_num+=1
      link_chain = child.get('source').split('-')
      for page in link_chain:
        if not page.startswith('X'):
          if not page.startswith('DMA'): 
            dummy_dict[mapping_dict[page.split('.')[0]]] = 0
        else:
          dummy_dict[page] = 0

    used_pages_list = dummy_dict.keys()
    used_pages_list.sort()
    return used_pages_list



  # create ip directory for each page 
  def create_ip(self):

    mapping_dict = self.prflow_params['mapping_dict']
    links = self.prflow_params['links']
    self.used_pages = self.get_all_used_pages(links)
     

    # prepare all the files and directories for synthesis
    self.re_mkdir(self.mono_bft_dir+'/ip_repo')
    for page in self.used_pages:
    #for fun_num, fun_name in enumerate(self.prflow_params['syn_fun_list']):
      page_num = page
      fun_name = self.get_key(page, mapping_dict)
      fun_num = self.get_entry(fun_name,  self.prflow_params['syn_fun_list'])


      num_bram_addr_bits =int(self.prflow_params['bram_addr_bits'])
      self.re_mkdir(self.mono_bft_dir+'/ip_repo/'+page)
      self.write_lines(self.mono_bft_dir+'/ip_repo/'+page+'/ip_page.tcl', self.return_ip_page_tcl_list(fun_name, page_num))
      self.write_lines(self.mono_bft_dir+'/ip_repo/'+page+'/run.sh',      self.return_run_sh_list(self.prflow_params['Xilinx_dir'], 'ip_page.tcl'), True)
      self.write_lines(self.mono_bft_dir+'/ip_repo/'+page+'/qsub_run.sh', self.return_run_sh_list(self.prflow_params['qsub_Xilinx_dir'], 'ip_page.tcl'), True)
      self.cp_dir(self.static_dir+'/src/'+self.place_holder[page]+'.v', self.mono_bft_dir+'/ip_repo/'+page+'/pe_empty_'+page+'.v')

      replace_dict={'module pe_empty':'module pe_empty_'+page+'#('}
      self.replace_lines(self.mono_bft_dir+'/ip_repo/'+page+'/pe_empty_'+page+'.v', replace_dict) 

      if fun_name != 'DMA':
        width_list = self.prflow_params['datawidth_dict'][page]
        replace_dict = {
                    'parameter WEST_WIDTH': 'parameter WEST_WIDTH = '+str(width_list[0])+',',
                    'parameter NORTH_WIDTH': 'parameter NORTH_WIDTH = '+str(width_list[1])+',',
                    'parameter SOUTH_WIDTH': 'parameter SOUTH_WIDTH = '+str(width_list[2])+',',
                    'parameter EAST_WIDTH': 'parameter EAST_WIDTH = '+str(width_list[3])+','
                    }
        self.replace_lines(self.mono_bft_dir+'/ip_repo/'+page+'/pe_empty_'+page+'.v', replace_dict) 

    self.modify_pages(self.gen_routing_dict(links, mapping_dict), mapping_dict)
    
    input_num_list  = self.prflow_params['input_num_list']
    output_num_list = self.prflow_params['output_num_list'] 

  
    for page in self.used_pages:
      fun_name = self.get_key(page, mapping_dict)
      fun_num = self.get_entry(fun_name,  self.prflow_params['syn_fun_list'])
      if fun_name != 'switchbox':
        self.add_lines(self.mono_bft_dir+'/ip_repo/'+page+'/pe_empty_'+page+'.v', 'endmodule', self.return_function_inst_v_list(fun_name, input_num_list[fun_num], output_num_list[fun_num])) 
      self.add_lines(self.mono_bft_dir+'/ip_repo/'+page+'/pe_empty_'+page+'.v', 'endmodule', self.return_assignment_v_list(
                                                                                                                                              self.place_holder[page], 
                                                                                                                                              self.prflow_params['datawidth_dict'][page], 
                                                                                                                                              self.width_max_dict[page])) 



  def create_shell_file(self):
  # local run:
  #   main.sh <- |_ execute each run.sh <- ip_page.tcl
  #              |_ vivado project_syn2gen.tcl
  #              |_ vivado project_syn2bits.tcl
  #
  # qsub run:
  #   qsub_main.sh <-|_ Qsubmit each qsub_run.sh <- ip_page.tcl
  #                  |_ qsub_project_syn2gen.sh <- project_syn2gen.tcl
  #                  |_ qsub_sub_syn.sh <-|_ go through very synthesis directies and Qsubmmit jobs <- runme.sh
  #                                       |_ qsub_project_syn2bits.sh <- project_syn2bits.tcl
  #                                       |_ qsub_project_sdk <- project_xsdk_dma.tcl
   
    self.cp_dir(self.static_dir + '/project_syn2gen.tcl', self.mono_bft_dir+'/project_syn2gen.tcl')
    self.add_lines(self.mono_bft_dir+'/project_syn2gen.tcl', '# Create address segments', self.return_syn2gen_tcl_list_local())
    self.write_lines(self.mono_bft_dir+'/project_syn2bits.tcl', self.return_syn2bits_tcl_list(), False)
    replace_dict={'set Benchmark_name': "set Benchmark_name " + self.prflow_params['benchmark_name']}
    self.replace_lines(self.mono_bft_dir+'/project_xsdk_core.tcl', replace_dict)
    self.write_lines(self.mono_bft_dir+'/main.sh', self.return_main_sh_list_local(), True)
    self.write_lines(self.mono_bft_dir+'/qsub_main.sh', self.return_qsub_main_sh_list_local(), True)
    self.write_lines(self.mono_bft_dir+'/qsub_project_syn2gen.sh', self.return_run_sh_list(self.prflow_params['qsub_Xilinx_dir'], 'project_syn2gen.tcl'), True)    
    self.write_lines(self.mono_bft_dir+'/qsub_sub_syn.sh', self.return_sub_syn_sh_list_local(), True)
    self.write_lines(self.mono_bft_dir+'/qsub_project_syn2bits.sh', self.return_run_sh_list(self.prflow_params['qsub_Xilinx_dir'], 'project_syn2bits.tcl'), True)    
    self.write_lines(self.mono_bft_dir+'/qsub_project_xsdk.sh', self.return_run_sdk_sh_list_local(self.prflow_params['qsub_Xilinx_dir'], 'project_xsdk_core.tcl'), True)    


  def uncomment_page_empty(self):
    modification_dict = {'/*': '',
                         '*/': '',
                         'parameter PAYLOAD_BITS': 'parameter PAYLOAD_BITS = 32,',
                         'parameter NUM_IN_PORTS': 'parameter NUM_IN_PORTS = 4,',
                         'parameter NUM_OUT_PORTS': 'parameter NUM_OUT_PORTS = 4'}

    self.replace_lines(self.mono_bft_dir+'/src/page_empty.v', modification_dict)


  def run(self):
    # mk work directory
    if self.prflow_params['mono_bft_regen']=='1':
      self.re_mkdir(self.mono_bft_dir)
    
    # copy the hld/xdc files from static dirctory
    self.cp_dir(self.static_dir + '/src ', self.mono_bft_dir)
    self.cp_dir(self.static_dir + '/dummy_repo/*/*.v', self.mono_bft_dir+'/src/')

    # copy the xsdk tcl to local directory
    self.cp_dir('./input_files/script_src/project_xsdk_core.tcl ', self.mono_bft_dir)

    # enable the logic inside page, so that vivado can 
    # implement it
    self.uncomment_page_empty()

    # clear up the xdc file
    self.write_lines(self.mono_bft_dir+'/src/pbplock_40.xdc', [''])

    # generate shell files for qsub run and local run
    self.create_shell_file() 

    # create ip directories for all the pages
    self.create_ip()
    
    # go to the local mono_bft directory and run the qsub command
    # os.chdir(self.mono_bft_dir)
    # if self.prflow_params['run_qsub']:
    #   os.system('./qsub_main.sh')
    # os.chdir('../../')


 



