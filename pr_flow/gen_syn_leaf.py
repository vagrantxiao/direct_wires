# -*- coding: utf-8 -*-   

import os  
import subprocess
from gen_basic import gen_basic



class gen_syn_leaf_files(gen_basic):
   # parse the list number according to the direction
  # WEST:  0
  # NORTH: 1
  # SOUTH: 2
  # EAST:  3
  def return_direction_num(self, direction):
    direction_dict = {'W': 0, 'N': 1, 'S': 2, 'E': 3}
    return direction_dict[direction]
  
  
  # translate the function name to page name if necessary
  def return_page(self, str_in, mapping_dict):
    if str_in.startswith('X'):
      return str_in
    else:
      return mapping_dict[str_in]

   # return out_edge, in_edge
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
 

  # calculate the minumum datawidht for all the meash pages
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
       

    max_used_dict = {} 
    for key in width_in_dict: 
      value_in = width_in_dict[key]
      value_out = width_out_dict[key]
      value_max = value_in[:]
      for i in range(len(value_in)):
        if value_max[i] < value_out[i]:
          value_max[i] = value_out[i]
      max_used_dict[key] = value_max
  
    print "maximum used wires for all the pages"       
    keys_list = max_used_dict.keys()
    keys_list.sort() 
    for key in keys_list:
      print key, max_used_dict[key]

 
    return links_dict


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



  def return_In_inst_v_list(self, width, val_in, ready_upward, din, val_out, ready_downward, dout):
    local_list = []
    local_list.append('wire ['+str(width)+'-1:0] '+dout+';')
    local_list.append('wire '+val_out+';')
    local_list.append('wire '+ready_downward+';')
    local_list.append('')
    local_list.append('assign '+dout+' = '+din+';')
    local_list.append('assign '+val_out+' = ' + val_in+';')
    local_list.append('assign '+ready_upward+' = '+ready_downward+';')
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
          Stream_v_list.append(self.return_Out_inst_v_list(value_local[0][2], 'Output_'+str(port_num+1)+'_V_V_ap_vld', 'Output_'+str(port_num+1)+'_V_V_ap_ack', 'Output_'+str(port_num+1)+'_V_V', value_local[0][5], value_local[0][6], value_local[0][7]))
        else :
        # This page is the destination page
          port_num = int(value_local[0][4].split('->')[1].split('.')[1])
          Stream_v_list.append(self.return_StreamIn_inst_v_list(key_local, value_local[0][2], value_local[0][5], value_local[0][6], value_local[0][7], 'Input_'+str(port_num+1)+'_V_V_ap_vld',  'Input_'+str(port_num+1)+'_V_V_ap_ack','Input_'+str(port_num+1)+'_V_V'))

      for lines_list in RelayStation_v_list:
        self.add_lines(self.syn_dir+'/'+page+'/'+self.place_holder[page]+'.v', 'endmodule',  lines_list) 
      for lines_list in Stream_v_list:
        if fun_name != 'DMA':
          self.add_lines(self.syn_dir+'/'+page+'/'+self.place_holder[page]+'.v', 'endmodule',  lines_list) 
 
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

  def get_key(self, val, my_dict): 
    for key, value in my_dict.items(): 
      if val == value: 
        return key
    return 'switchbox'

  def get_entry(self, val, my_list):
    for i in range(len(my_list)): 
      if val == my_list[i]:
        return i 

 
  # create one directory for each page 
  def create_page(self):
    mapping_dict = self.prflow_params['mapping_dict']
    links = self.prflow_params['links']
    self.used_pages = self.get_all_used_pages(links)
     

    # prepare all the files and directories for synthesis
    for page in self.used_pages:
      fun_name = self.get_key(page, mapping_dict)
      fun_num = self.get_entry(fun_name,  self.prflow_params['syn_fun_list'])
      #input_num = self.prflow_params['input_num_list'][fun_num]
      #output_num = self.prflow_params['output_num_list'][fun_num]
      num_bram_addr_bits =int(self.prflow_params['bram_addr_bits'])

      self.re_mkdir(self.syn_dir+'/'+page)

      self.write_lines(self.syn_dir+'/'+page+'/syn_page.tcl', self.return_syn_page_tcl_list(fun_name, self.place_holder[page]))
      self.write_lines(self.syn_dir+'/'+page+'/qsub_run.sh', self.return_run_sh_list(self.prflow_params['qsub_Xilinx_dir'], 'syn_page.tcl'), True)
      self.cp_dir(self.static_dir+'/src/'+self.place_holder[page]+'.v', self.syn_dir+'/'+page)
      if fun_name != 'DMA':
        width_list = self.prflow_params['datawidth_dict'][page]
        replace_dict = {
                    'parameter WEST_WIDTH': 'parameter WEST_WIDTH = '+str(width_list[0])+',',
                    'parameter NORTH_WIDTH': 'parameter NORTH_WIDTH = '+str(width_list[1])+',',
                    'parameter SOUTH_WIDTH': 'parameter SOUTH_WIDTH = '+str(width_list[2])+',',
                    'parameter EAST_WIDTH': 'parameter EAST_WIDTH = '+str(width_list[3])+','
                    }
        self.replace_lines(self.syn_dir+'/'+page+'/'+self.place_holder[page]+'.v', replace_dict) 



    self.modify_pages(self.gen_routing_dict(links, mapping_dict), mapping_dict)
    
    input_num_list  = self.prflow_params['input_num_list']
    output_num_list = self.prflow_params['output_num_list'] 

  
    for page in self.used_pages:
      fun_name = self.get_key(page, mapping_dict)
      fun_num = self.get_entry(fun_name,  self.prflow_params['syn_fun_list'])
      if fun_name != 'switchbox':
        self.add_lines(self.syn_dir+'/'+page+'/'+self.place_holder[page]+'.v', 'endmodule', self.return_function_inst_v_list(fun_name, input_num_list[fun_num], output_num_list[fun_num])) 
      self.add_lines(self.syn_dir+'/'+page+'/'+self.place_holder[page]+'.v', 'endmodule', self.return_assignment_v_list(
                                                                                                                                              self.place_holder[page], 
                                                                                                                                              self.prflow_params['datawidth_dict'][page], 
                                                                                                                                              self.width_max_dict[page])) 



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





  # main.sh will be used for local compilation
  def return_qsub_main_sh_list_local(self):
    lines_list = []
    lines_list.append('#!/bin/bash -e')
    # compile the IP for each page
    #for fun_name in self.prflow_params['syn_fun_list']:
    for page in self.used_pages:
      lines_list.append('cd ./' + page) 
      lines_list.append(self.return_qsub_command_str('./qsub_run.sh', 'hls_'+self.get_key(page, self.prflow_params['mapping_dict']), 'syn_'+page, self.prflow_params['MEM'], self.prflow_params['qsub_grid'], self.prflow_params['email'], self.prflow_params['qsub_Nodes']))
      lines_list.append('cd -') 

    return lines_list


  # main.sh will be used for local compilation
  def return_main_sh_list_local(self):
    lines_list = []
    lines_list.append('#!/bin/bash -e')
    lines_list.append('source '+self.prflow_params['Xilinx_dir'])
    # compile the IP for each page
    #for fun_name in self.prflow_params['syn_fun_list']:
    iter_num = 1
    for page in self.used_pages:
      lines_list.append('cd ./' + page) 
      if iter_num % int(self.prflow_params['jobNum']) == 0:
        lines_list.append('vivado -mode batch -source ./syn_page.tcl') 
      elif iter_num == len(self.used_pages):
        lines_list.append('vivado -mode batch -source ./syn_page.tcl') 
      else:
        lines_list.append('vivado -mode batch -source ./syn_page.tcl&') 
      lines_list.append('cd -') 
      iter_num += 1

    return lines_list


  def create_shell_file(self):
  # local run:
  #   main.sh <- |_ execute each run.sh <- syn_page.tcl
  #
  # qsub run:
  #   qsub_main.sh <-|_ Qsubmit each qsub_run.sh <- syn_page.tcl
   
    self.write_lines(self.syn_dir+'/main.sh', self.return_main_sh_list_local(), True)
    self.write_lines(self.syn_dir+'/qsub_main.sh', self.return_qsub_main_sh_list_local(), True)



  def run(self):

    # mk work directory
    if self.prflow_params['leaf_syn_regen']=='1':
      self.re_mkdir(self.syn_dir)
    
    # create ip directories for all the pages
    self.create_page()

    # generate shell files for qsub run and local run
    self.create_shell_file() 

    # create ip directories for all the nets
    # self.create_net()
    
    # go to the local mono_bft directory and run the qsub command
    os.chdir(self.syn_dir)
    if self.prflow_params['run_qsub']:
      os.system('./qsub_main.sh')
    os.chdir('../../')


 



