# -*- coding: utf-8 -*-


import os
import subprocess
import xml.etree.ElementTree
import re

class gen_basic:
  def __init__(self, prflow_params):
    self.prflow_params = prflow_params
    self.bft_dir = self.prflow_params['workspace']+'/F000_bft_gen'
    self.static_dir = self.prflow_params['workspace']+'/F001_static_' + self.prflow_params['nl'] + '_leaves'
    self.hls_dir = self.prflow_params['workspace']+'/F002_hls_'+self.prflow_params['benchmark_name']
    self.syn_dir = self.prflow_params['workspace']+'/F003_syn_'+self.prflow_params['benchmark_name']
    self.pr_dir = self.prflow_params['workspace']+'/F004_pr_'+self.prflow_params['benchmark_name']
    self.mono_bft_dir = self.prflow_params['workspace']+'/F007_mono_bft_'+self.prflow_params['benchmark_name']
    self.hls_dir = self.prflow_params['workspace']+'/F002_hls_'+self.prflow_params['benchmark_name']
    self.net_list = ['1', '1', '1', '1', '1', '2', '2', '2',
                     '2', '2', '2', '0', '3', '3', '3', '3',
                     '3', '3', '4', '4', '4', '4', '4', '4',
                     '5', '5', '5', '5', '5', '5', '5', '5']
    self.place_holder = prflow_params['place_holder_dict']


  ######################################################################################################################################################
  # shell functions start

  # return the qsub command according to the input parameters
  def return_qsub_command_str(self, shell_file='./qsub_run', hold_jid='NONE', name='NONE', MEM='2G', q='70s', email='qsub@qsub.com', node_num='1'):
    # return ('qsub -N '+name + ' -q ' + q + ' -hold_jid ' + hold_jid + ' -m abe -M ' + email + ' -l mem='+MEM + ' -pe onenode '+node_num + '  -cwd '+ shell_file)
    return ('sbatch --ntasks=1 --cpus-per-task=8 --mem-per-cpu=7500mb --job-name='+name\
            +' --dependency=$(squeue --noheader --format %i --user=$USER --name '+hold_jid +'| sed -n -e \'H;${x;s/\\n/,/g;s/^,//;p;}\') '+shell_file)
  def get_file_name(self, file_dir):
  # return a file list under a file_dir
    for root, dirs, files in os.walk(file_dir):
      return files

  def replace_lines(self, filename, modification_dict):
  # change the string(key of modification_dict) to
  # target string (value of modification_dict)
    try:
      file_in =  open(filename, 'r')
      file_out = open(filename+'tmp', 'w')
      for line in file_in:
        find_target = False
        for key, value in modification_dict.items():
          if line.replace(key, '') != line:
            file_out.write(value+'\n')
            find_target = True
            break
        if find_target == False:
          file_out.write(line)
      file_out.close()
      file_in.close()
      os.system('mv '+filename+'tmp '+filename)
    except:
      print "Modification for "+filename+" failed!"

  def add_lines(self, filename, anchor, lines_list):
  # add more lines in a file according to
  # some anchor string
    try:
      file_in =  open(filename, 'r')
      file_out = open(filename+'tmp', 'w')
      for line in file_in:
        if line.replace(anchor, '') != line:
          file_out.write('\n'.join(lines_list)+'\n')
        file_out.write(line)
      file_out.close()
      file_in.close()
      os.system('mv '+filename+'tmp '+filename)
    except:
      print "Adding more line in "+filename+" failed!"

  def write_lines(self, filename, lines_list, executable=False, write_or_add='w'):
    try:
      file_out = open(filename, write_or_add)
      file_out.write('\n'.join(lines_list)+'\n')
      file_out.close()
      if executable == True:
         os.system('chmod +x ' + filename)
    except:
      print "Writing "+filename+" failed!"

  def re_mkdir(self, dir_name):
     os.system('rm -rf ' + dir_name)
     os.system('mkdir ' + dir_name)

  def cp_dir(self, src_dir, dst_dir):
     os.system('cp -rf '+src_dir+' '+dst_dir)

  def return_run_sh_list(self, vivado_dir, tcl_file):
    return ([
      '#!/bin/bash -e',
      'source ' + vivado_dir,
      'vivado -mode batch -source ' + tcl_file,
      ''])

  def return_main_sh_list(self, run_file='run.sh'):
    return ([
      '#!/bin/bash -e',
      './' + run_file,
      ''])

  # shell functions end
  ######################################################################################################################################################





  ######################################################################################################################################################
  # verilog functions start

  # parse the interconnection according to the input xml file
  def parse_connection(self):
    root = xml.etree.ElementTree.parse(self.prflow_params['input_file_name'])
    links = root.findall('link')
    conn_list = []
    for child in links:
      func_name, port_num = child.get('source').split('.')
      page_num = self.prflow_params['page_list'][self.prflow_params['syn_fun_list'].index(func_name)].replace('Function','') if func_name != 'DMA' else '1'
      src = page_num+'.'+port_num
      func_name, port_num = child.get('destination').split('.')
      page_num = self.prflow_params['page_list'][self.prflow_params['syn_fun_list'].index(func_name)].replace('Function','') if func_name != 'DMA' else '1'
      dest =  page_num+'.'+port_num
      conn_list.append((src, dest))
    return conn_list

  # convert the a list to an accumulation form
  # eg.:    in_list = [1, 2, 3, 4, 5]
  # output: [1, 3, 6, 10, 15]
  def accum(self, in_list, index=1):
    if index == len(in_list):
      return
    else:
      in_list[index]+=in_list[index-1]
      self.accum(in_list, index+1)

  def return_dummy_page_v_list(self, data_width, port_num):
    num_bram_addr_bits = 7
    # print verilog header
    lines_list = []
    lines_list.append('module page_'+str(data_width)+'_'+str(port_num)+' #(')
    lines_list.append('parameter PAYLOAD_BITS = '+str(data_width)+',')
    lines_list.append('parameter NUM_IN_PORTS = '+str(port_num)+',')
    lines_list.append('parameter NUM_OUT_PORTS = '+str(port_num)+',')
    lines_list.append('parameter NUM_BRAM_ADDR_BITS = '+str(num_bram_addr_bits)+'')
    lines_list.append(')(')
    lines_list.append('input wire clk,')
    lines_list.append('input wire reset,')
    lines_list.append('input wire ap_start,')
    lines_list.append('input wire [PAYLOAD_BITS*NUM_IN_PORTS-1:0] din,')
    lines_list.append('input wire [NUM_IN_PORTS-1:0] val_in,')
    lines_list.append('output wire [NUM_IN_PORTS-1:0] ready_upward,')
    lines_list.append('output wire [PAYLOAD_BITS*NUM_OUT_PORTS-1:0] dout,')
    lines_list.append('output wire [NUM_OUT_PORTS-1:0] val_out,')
    lines_list.append('input wire [NUM_OUT_PORTS-1:0] ready_downward')
    lines_list.append(');')
    lines_list.append('')

    # instantiate input shell
    for i in range(int(port_num)):
      lines_list.append('wire [PAYLOAD_BITS-1:0] Input_'+str(i+1)+'_V_V;')
      lines_list.append('wire Input_'+str(i+1)+'_V_V_ap_vld;')
      lines_list.append('wire Input_'+str(i+1)+'_V_V_ap_ack;')
      lines_list.append('stream_shell #(')
      lines_list.append('    .PAYLOAD_BITS(PAYLOAD_BITS),')
      lines_list.append('    .NUM_BRAM_ADDR_BITS(NUM_BRAM_ADDR_BITS)')
      lines_list.append(') stream_in_'+str(i+1)+'(')
      lines_list.append('    .clk_out(clk),')
      lines_list.append('    .reset_out(reset),')
      lines_list.append('    .din(din[PAYLOAD_BITS*'+str(i+1)+'-1:PAYLOAD_BITS*'+str(i)+']),')
      lines_list.append('    .val_in(val_in['+str(i)+']),')
      lines_list.append('    .ready_upward(ready_upward['+str(i)+']),')
      lines_list.append('    .dout(Input_'+str(i+1)+'_V_V),')
      lines_list.append('    .val_out(Input_'+str(i+1)+'_V_V_ap_vld),')
      lines_list.append('    .ready_downward(Input_'+str(i+1)+'_V_V_ap_ack));')
      lines_list.append('')

      lines_list.append('wire [PAYLOAD_BITS-1:0] Output_'+str(i+1)+'_V_V;')
      lines_list.append('wire Output_'+str(i+1)+'_V_V_ap_vld;')
      lines_list.append('wire Output_'+str(i+1)+'_V_V_ap_ack;')
      lines_list.append('stream_shell #(')
      lines_list.append('    .PAYLOAD_BITS(PAYLOAD_BITS),')
      lines_list.append('    .NUM_BRAM_ADDR_BITS(NUM_BRAM_ADDR_BITS)')
      lines_list.append(') stream_out_'+str(i+1)+'(')
      lines_list.append('    .clk_out(clk),')
      lines_list.append('    .reset_out(reset),')
      lines_list.append('    .din(Output_'+str(i+1)+'_V_V),')
      lines_list.append('    .val_in(Output_'+str(i+1)+'_V_V_ap_vld),')
      lines_list.append('    .ready_upward(Output_'+str(i+1)+'_V_V_ap_ack),')
      lines_list.append('    .dout(dout[PAYLOAD_BITS*'+str(i+1)+'-1:PAYLOAD_BITS*'+str(i)+']),')
      lines_list.append('    .val_out(val_out['+str(i)+']),')
      lines_list.append('    .ready_downward(ready_downward['+str(i)+']));')
      lines_list.append('')

      # instantiate user functions
      lines_list.append('user_kernel#(')
      lines_list.append('  .PAYLOAD_BITS(PAYLOAD_BITS)')
      lines_list.append(')user_kernel'+str(i+1)+'(')
      lines_list.append('  .ap_clk(clk),')
      lines_list.append('  .ap_rst(reset),')
      lines_list.append('  .ap_start(ap_start),')
      lines_list.append('  .ap_done(),')
      lines_list.append('  .ap_idle(),')
      lines_list.append('  .Input_1_V_V(Input_'+str(i+1)+'_V_V),')
      lines_list.append('  .Input_1_V_V_ap_vld(Input_'+str(i+1)+'_V_V_ap_vld),')
      lines_list.append('  .Input_1_V_V_ap_ack(Input_'+str(i+1)+'_V_V_ap_ack),')
      lines_list.append('  .Output_1_V_V(Output_'+str(i+1)+'_V_V),')
      lines_list.append('  .Output_1_V_V_ap_vld(Output_'+str(i+1)+'_V_V_ap_vld),')
      lines_list.append('  .Output_1_V_V_ap_ack(Output_'+str(i+1)+'_V_V_ap_ack),')
      lines_list.append('  .ap_ready()')
      lines_list.append('  );')


    lines_list.append('endmodule')
    return lines_list



  def return_page_v_list(self, fun_num, fun_name, input_num, output_num, for_syn=False):
    data_width, port_num = self.prflow_params['page'+str(fun_num)+'_net']
    num_bram_addr_bits = self.prflow_params['bram_addr_bits']
    dw_out = self.prflow_params['datawidth_out_list'][int(fun_num)]
    dw_in  = self.prflow_params['datawidth_in_list'][int(fun_num)]

    wires_out_used = dw_out[:]
    self.accum(wires_out_used)
    wires_in_used = dw_in[:]
    self.accum(wires_in_used)

    # print verilog header
    lines_list = []
    if for_syn:
      lines_list.append('module page #(')
    else:
      lines_list.append('module page_'+str(fun_num)+' #(')
    lines_list.append('parameter PAYLOAD_BITS = '+str(data_width)+',')
    lines_list.append('parameter NUM_IN_PORTS = '+str(port_num)+',')
    lines_list.append('parameter NUM_OUT_PORTS = '+str(port_num)+',')
    lines_list.append('parameter NUM_BRAM_ADDR_BITS = '+str(num_bram_addr_bits)+'')
    lines_list.append(')(')
    lines_list.append('input wire clk,')
    lines_list.append('input wire reset,')
    lines_list.append('input wire ap_start,')
    lines_list.append('input wire [PAYLOAD_BITS*NUM_IN_PORTS-1:0] din,')
    lines_list.append('input wire [NUM_IN_PORTS-1:0] val_in,')
    lines_list.append('output wire [NUM_IN_PORTS-1:0] ready_upward,')
    lines_list.append('output wire [PAYLOAD_BITS*NUM_OUT_PORTS-1:0] dout,')
    lines_list.append('output wire [NUM_OUT_PORTS-1:0] val_out,')
    lines_list.append('input wire [NUM_OUT_PORTS-1:0] ready_downward')
    lines_list.append(');')
    lines_list.append('')

    # instantiate input shell
    for i in range(int(input_num)):
      lines_list.append('wire ['+str(dw_in[i])+'-1:0] Input_'+str(i+1)+'_V_V;')
      lines_list.append('wire Input_'+str(i+1)+'_V_V_ap_vld;')
      lines_list.append('wire Input_'+str(i+1)+'_V_V_ap_ack;')
      lines_list.append('stream_shell #(')
      lines_list.append('    .PAYLOAD_BITS('+str(dw_in[i])+'),')
      lines_list.append('    .NUM_BRAM_ADDR_BITS(NUM_BRAM_ADDR_BITS)')
      lines_list.append(') stream_in_'+str(i+1)+'(')
      lines_list.append('    .clk(clk),')
      lines_list.append('    .reset(reset),')
      lines_list.append('    .din(din['+str(wires_in_used[i])+'-1:'+str(wires_in_used[i]-dw_in[i])+']),')
      lines_list.append('    .val_in(val_in['+str(i)+']),')
      lines_list.append('    .ready_upward(ready_upward['+str(i)+']),')
      lines_list.append('    .dout(Input_'+str(i+1)+'_V_V),')
      lines_list.append('    .val_out(Input_'+str(i+1)+'_V_V_ap_vld),')
      lines_list.append('    .ready_downward(Input_'+str(i+1)+'_V_V_ap_ack));')
      lines_list.append('')
      # lines_list.append('wire [31:0] cnt_in_'+str(i)+';')
      # lines_list.append('stream_cnt #(')
      # lines_list.append('    .CNT_WIDTH(32)')
      # lines_list.append(')strean_in_'+str(i+1)+'_cnt(')
      # lines_list.append('    .clk(clk_user),')
      # lines_list.append('    .reset(reset_user),')
      # lines_list.append('    .valid(Input_'+str(i+1)+'_V_V_ap_vld),')
      # lines_list.append('    .ready(Input_'+str(i+1)+'_V_V_ap_ack),')
      # lines_list.append('    .cnt(cnt_in_'+str(i)+')')
      # lines_list.append(');')
      lines_list.append('')

    # instantiate output shell
    for i in range(int(output_num)):
      lines_list.append('wire ['+str(dw_out[i])+'-1:0] Output_'+str(i+1)+'_V_V;')
      lines_list.append('wire Output_'+str(i+1)+'_V_V_ap_vld;')
      lines_list.append('wire Output_'+str(i+1)+'_V_V_ap_ack;')
      lines_list.append('stream_shell #(')
      lines_list.append('    .PAYLOAD_BITS('+str(dw_out[i])+'),')
      lines_list.append('    .NUM_BRAM_ADDR_BITS(NUM_BRAM_ADDR_BITS)')
      lines_list.append(') stream_out_'+str(i+1)+'(')
      lines_list.append('    .clk(clk),')
      lines_list.append('    .reset(reset),')
      lines_list.append('    .din(Output_'+str(i+1)+'_V_V),')
      lines_list.append('    .val_in(Output_'+str(i+1)+'_V_V_ap_vld),')
      lines_list.append('    .ready_upward(Output_'+str(i+1)+'_V_V_ap_ack),')
      lines_list.append('    .dout(dout['+str(wires_out_used[i])+'-1:'+str(wires_out_used[i]-dw_out[i])+']),')
      lines_list.append('    .val_out(val_out['+str(i)+']),')
      lines_list.append('    .ready_downward(ready_downward['+str(i)+']));')
      lines_list.append('')
      # lines_list.append('wire [31:0] cnt_out_'+str(i)+';')
      # lines_list.append('stream_cnt #(')
      # lines_list.append('    .CNT_WIDTH(32)')
      # lines_list.append(')strean_out_'+str(i+1)+'_cnt(')
      # lines_list.append('    .clk(clk_user),')
      # lines_list.append('    .reset(reset_user),')
      # lines_list.append('    .valid(Output_'+str(i+1)+'_V_V_ap_vld),')
      # lines_list.append('    .ready(Output_'+str(i+1)+'_V_V_ap_ack),')
      # lines_list.append('    .cnt(cnt_out_'+str(i)+')')
      # lines_list.append(');')
      lines_list.append('')
    
    # lines_list.append('reg ap_start;')
    # lines_list.append('always@(posedge clk_user) begin')
    # lines_list.append('  if(reset_user)')
    # lines_list.append('    ap_start <= 0;')
    # lines_list.append('  else if (Input_1_V_V_ap_vld)')
    # lines_list.append('    ap_start <= 1;')
    # lines_list.append('  else')
    # lines_list.append('    ap_start <= ap_start;')
    # lines_list.append('end')
    # lines_list.append('')

    # instantiate user functions
    lines_list.append('    '+fun_name + ' ' + fun_name+'_inst(')
    lines_list.append('        .ap_clk(clk),')
    lines_list.append('        .ap_rst(reset),')
    lines_list.append('        .ap_start(ap_start),')
    lines_list.append('        .ap_done(),')
    lines_list.append('        .ap_idle(ap_idle),')
    for i in range (1, int(input_num)+1):
      lines_list.append('        .Input_'+str(i)+'_V_V(Input_'+str(i)+'_V_V),')
      lines_list.append('        .Input_'+str(i)+'_V_V_ap_vld(Input_'+str(i)+'_V_V_ap_vld),')
      lines_list.append('        .Input_'+str(i)+'_V_V_ap_ack(Input_'+str(i)+'_V_V_ap_ack),')
    for i in range (1, int(output_num)+1):
      lines_list.append('        .Output_'+str(i)+'_V_V(Output_'+str(i)+'_V_V),')
      lines_list.append('        .Output_'+str(i)+'_V_V_ap_vld(Output_'+str(i)+'_V_V_ap_vld),')
      lines_list.append('        .Output_'+str(i)+'_V_V_ap_ack(Output_'+str(i)+'_V_V_ap_ack),')
    lines_list.append('        .ap_ready(ap_ready)')
    lines_list.append('        );  ')

    # assign unused outputs to GND
    if wires_out_used[int(output_num)-1] < int(data_width)*int(port_num):
      lines_list.append('assign dout[PAYLOAD_BITS*NUM_IN_PORTS-1:'+str(wires_out_used[int(output_num)-1])+']=0;')
    if int(input_num) < int(port_num):
      lines_list.append('assign ready_upward['+str(port_num)+'-1:'+str(input_num)+']=0;')
    if int(output_num) < int(port_num):
      lines_list.append('assign val_out['+str(port_num)+'-1:'+str(output_num)+']=0;')

    lines_list.append('endmodule')
    return lines_list

  # return the verilog module definition
  def return_v_header_list(self, name, page_num_list=range(3), net_num_list=range(1,5), out_type='wire'):
    lines_list = []
    lines_list.append('module '+name+'#(')
    lines_list.extend(['parameter PAGE'+str(i)+'_PAYLOAD_BITS = '+self.prflow_params['page'       +str(i)+'_net'][0]+',' for i in page_num_list])
    lines_list.extend(['parameter NET'+str(i)+ '_PAYLOAD_BITS = '+self.prflow_params['net0_'+'net'+str(i)][0]+','        for i in net_num_list])
    lines_list.extend(['parameter PAGE'+str(i)+'_PORT_NUM = '    +self.prflow_params['page'       +str(i)+'_net'][1]+',' for i in page_num_list])
    lines_list.extend(['parameter NET'+str(i)+ '_PORT_NUM = '    +self.prflow_params['net0_'+'net'+str(i)][1]+',' for i in net_num_list])
    lines_list.append('parameter WIDTH_DUMMY=0')
    lines_list.append(')(')
    lines_list.append('input clk,')

    # write the stream ports for page
    for i in page_num_list:
      lines_list.append('input wire [PAGE'+str(i)+'_PAYLOAD_BITS*PAGE'+str(i)+'_PORT_NUM-1:0] page'+str(i)+'_din,')
      lines_list.append('input wire [PAGE'+str(i)+'_PORT_NUM-1:0] page'+str(i)+'_val_in,')
      lines_list.append('output '+out_type+' [PAGE'+str(i)+'_PORT_NUM-1:0] page'+str(i)+'_ready_upward,')
      lines_list.append('')
      lines_list.append('output '+out_type+' [PAGE'+str(i)+'_PAYLOAD_BITS*PAGE'+str(i)+'_PORT_NUM-1:0] page'+str(i)+'_dout,')
      lines_list.append('output '+out_type+' [PAGE'+str(i)+'_PORT_NUM-1:0] page'+str(i)+'_val_out,')
      lines_list.append('input wire [PAGE'+str(i)+'_PORT_NUM-1:0] page'+str(i)+'_ready_downward,')
      lines_list.append('')

    # write the stream ports for net
    for i in net_num_list:
      lines_list.append('input wire [NET'+str(i)+'_PAYLOAD_BITS*NET'+str(i)+'_PORT_NUM-1:0] net'+str(i)+'_din,')
      lines_list.append('input wire [NET'+str(i)+'_PORT_NUM-1:0] net'+str(i)+'_val_in,')
      lines_list.append('output '+out_type+' [NET'+str(i)+'_PORT_NUM-1:0] net'+str(i)+'_ready_upward,')
      lines_list.append('')
      lines_list.append('output '+out_type+' [NET'+str(i)+'_PAYLOAD_BITS*NET'+str(i)+'_PORT_NUM-1:0] net'+str(i)+'_dout,')
      lines_list.append('output '+out_type+' [NET'+str(i)+'_PORT_NUM-1:0] net'+str(i)+'_val_out,')
      lines_list.append('input wire [NET'+str(i)+'_PORT_NUM-1:0] net'+str(i)+'_ready_downward,')
      lines_list.append('')
    lines_list.append('input reset);')
    return lines_list


  def return_inst_RelayStation_v_list(self, relay_datawidth, src, dest, pp_start_num, pp_end_num, src_bus=(31,0,0), dest_bus=(31,0,0)):
  # return the relay station instantiations
  # relay_datawidth: the datawidth for the asynchronous fifo
  # src: the source stream name
  # dest: the destination stream name
  # the pipeline register start num
  # the pipeline register end num
  # src_bus = (most significant bit, least significant bit num, port num)
  #   eg.: src_bus = (63, 32, 1) the source stream is : din[63:32], val_in[1], ready_upward[1]
  # dest_bus = (most significant bit, least significant bit num, port num)
  #   eg.: dest_bus = (63, 32, 1) the destination stream is : dout[63:32], val_out[1], ready_downward[1]
    lines_list = []
    for i in range(pp_start_num, pp_end_num):
      if i != pp_end_num:
        lines_list.append('  wire ['+str(relay_datawidth)+'-1:0] '+src+dest+'_'+str(i)+'_dout;')
        lines_list.append('  wire '+src+dest+'_'+str(i)+'_ready_downward;')
        lines_list.append('  wire '+src+dest+'_'+str(i)+'_val_out;')
      lines_list.append('  RelayStation #(')
      lines_list.append('    .PAYLOAD_BITS('+str(relay_datawidth)+')')
      lines_list.append('  )RS_'+src+'_'+dest+'_'+str(i)+'(')
      lines_list.append('    .clk(clk),')
      if i == pp_start_num:
        lines_list.append('    .din('+src+'_din['+str(src_bus[0])+':'+str(src_bus[1])+']),')
        lines_list.append('    .val_in('+src+'_val_in['+str(src_bus[2])+']),')
        lines_list.append('    .ready_upward('+src+'_ready_upward['+str(src_bus[2])+']),')
      else:
        lines_list.append('    .din('+src+dest+'_'+str(i-1)+'_dout),')
        lines_list.append('    .val_in('+src+dest+'_'+str(i-1)+'_val_out),')
        lines_list.append('    .ready_upward('+src+dest+'_'+str(i-1)+'_ready_downward),')

      if i != pp_end_num-1:
        lines_list.append('    .dout('+src+dest+'_'+str(i)+'_dout),')
        lines_list.append('    .val_out('+src+dest+'_'+str(i)+'_val_out),')
        lines_list.append('    .ready_downward('+src+dest+'_'+str(i)+'_ready_downward),')
      else:
        lines_list.append('    .dout('+dest+'_dout['+str(dest_bus[0])+':'+str(dest_bus[1])+']),')
        lines_list.append('    .val_out('+dest+'_val_out['+str(dest_bus[2])+']),')
        lines_list.append('    .ready_downward('+dest+'_ready_downward['+str(dest_bus[2])+']),')
      lines_list.append('    .reset(reset));\n')
    return lines_list

  def return_stream_p2p_v_list(self, page_num_list=range(3), net_num_list=range(1,5)):
    lines_list = []
    lines_list.append('always@(posedge clk) begin')
    lines_list.append('  if (reset) begin')
    lines_list.extend(['    page'+str(i)+'_dout <= 0;' for i in page_num_list])
    lines_list.extend(['    page'+str(i)+'_val_out <= 0;' for i in page_num_list])
    lines_list.extend(['    page'+str(i)+'_ready_upward <= 0;' for i in page_num_list])
    lines_list.extend(['    net'+str(i)+'_dout <= 0;' for i in net_num_list])
    lines_list.extend(['    net'+str(i)+'_val_out <= 0;' for i in net_num_list])
    lines_list.extend(['    net'+str(i)+'_ready_upward <= 0;' for i in net_num_list])
    lines_list.append('  end else begin')
    lines_list.extend(['    page'+str(i)+'_dout <= page'+str(i)+'_din;' for i in page_num_list])
    lines_list.extend(['    page'+str(i)+'_val_out <= page'+str(i)+'_val_in;' for i in page_num_list])
    lines_list.extend(['    page'+str(i)+'_ready_upward <= page'+str(i)+'_ready_downward;' for i in page_num_list])
    lines_list.extend(['    net'+str(i)+'_dout <= net'+str(i)+'_din;' for i in net_num_list])
    lines_list.extend(['    net'+str(i)+'_val_out <= net'+str(i)+'_val_in;' for i in net_num_list])
    lines_list.extend(['    net'+str(i)+'_ready_upward <= net'+str(i)+'_ready_downward;' for i in net_num_list])
    lines_list.append('  end')
    lines_list.append('end')
    return lines_list



  # generate dummy pin2pin registers between input and output stream
  def gen_net_dummy(self):
    # net feature dictionary
    nd = {}

    # add verilog module defination for each network verilog file
    nd['net0'] = self.return_v_header_list('net0', [11],          [1,2,3,4,5], 'reg')
    nd['net1'] = self.return_v_header_list('net1', range(5),      [1], 'reg')
    nd['net2'] = self.return_v_header_list('net2', range(5, 11),  [2], 'reg')
    nd['net3'] = self.return_v_header_list('net3', range(12, 18), [3], 'reg')
    nd['net4'] = self.return_v_header_list('net4', range(18, 24), [4], 'reg')
    nd['net5'] = self.return_v_header_list('net5', range(24, 32), [5], 'reg')

    nd['net0'].extend(self.return_stream_p2p_v_list([11],          [1,2,3,4,5]))
    nd['net1'].extend(self.return_stream_p2p_v_list(range(5),      [1]))
    nd['net2'].extend(self.return_stream_p2p_v_list(range(5, 11),  [2]))
    nd['net3'].extend(self.return_stream_p2p_v_list(range(12, 18), [3]))
    nd['net4'].extend(self.return_stream_p2p_v_list(range(18, 24), [4]))
    nd['net5'].extend(self.return_stream_p2p_v_list(range(24, 32), [5]))

    for i in range(6): nd['net'+str(i)].append('endmodule')
    return nd

  def accumulate_wire_num(self, list_in, num):
    sum = 0
    for i in range(num):
      sum += int(list_in[i])
    
    return sum
    
    

  # generate the direct interconnect verilog code
  def gen_routing(self):
    # net feature dictionary
    nd = {}

    # add verilog module defination for each network verilog file
    nd['net0'] = self.return_v_header_list('net0', [11],          [1,2,3,4,5])
    nd['net1'] = self.return_v_header_list('net1', range(5),      [1])
    nd['net2'] = self.return_v_header_list('net2', range(5, 11),  [2])
    nd['net3'] = self.return_v_header_list('net3', range(12, 18), [3])
    nd['net4'] = self.return_v_header_list('net4', range(18, 24), [4])
    nd['net5'] = self.return_v_header_list('net5', range(24, 32), [5])

    # initialize the counter for each pipeline registers for 6 network
    for i in range(6): nd['pp'+str(i)] = 0

    # the pipeline register lenth
    pp_center_unit = int(self.prflow_params['pp_0']) 
    pp_net_unit = int(self.prflow_params['pp_1'])

    # parse interconnection
    conn_list = self.parse_connection()

    # parse input datawidth for all the pages
    dw_in_list = self.prflow_params['datawidth_in_list']

    # parse output datawidth for all the pages
    dw_out_list = self.prflow_params['datawidth_out_list']

    # initialize the used wires and ports.
    # wires for dout
    # ports for ready_upward and val_out
    nd['in_center_wires_used']  = [0 for index in range(6)]
    nd['in_center_ports_used']  = [0 for index in range(6)]
    nd['out_center_wires_used'] = [0 for index in range(6)]
    nd['out_center_ports_used'] = [0 for index in range(6)]
    nd['in_page_wires_used'] = [0 for index in range(2**int(self.prflow_params['addr_bits']))]
    nd['in_page_ports_used'] = [0 for index in range(2**int(self.prflow_params['addr_bits']))]
    nd['out_page_wires_used'] = [0 for index in range(2**int(self.prflow_params['addr_bits']))]
    nd['out_page_ports_used'] = [0 for index in range(2**int(self.prflow_params['addr_bits']))]

    for (src, dest) in conn_list:
      src_page_num = int(src.split('.')[0])
      src_port_num = int(src.split('.')[1])
      dest_page_num = int(dest.split('.')[0])
      dest_port_num = int(dest.split('.')[1])
      sub_tree_src =  self.net_list[src_page_num]
      sub_tree_dest = self.net_list[dest_page_num]
      same_sub_tree = (sub_tree_src == sub_tree_dest)

      if same_sub_tree:
        # if source and dest are in the same sub_tree
        # only add routing path in one subtree
        nd['net'+sub_tree_src].append('\n\n// ' + src + '=>' + dest)

        #def return_inst_RelayStation_v_list(self, Relay_datawidth, src, dest, pp_start_num, pp_end_num, src_bus=(31,0,0), dest_bus=(31,0,0)):

        src_bus_lsb = self.accumulate_wire_num(dw_out_list[src_page_num], src_port_num)
        src_bus_msb = src_bus_lsb+dw_out_list[src_page_num][src_port_num]-1
        src_bus_num = src_port_num
        dest_bus_lsb = self.accumulate_wire_num(dw_in_list[dest_page_num], dest_port_num)
        dest_bus_msb = dest_bus_lsb+dw_in_list[dest_page_num][dest_port_num]-1
        dest_bus_num = dest_port_num
        nd['net'+sub_tree_src].extend(self.return_inst_RelayStation_v_list(dw_out_list[src_page_num][src_port_num],
                                                                           'page'+str(src_page_num),
                                                                           'page'+str(dest_page_num),
                                                                           nd['pp'+sub_tree_src],
                                                                           nd['pp'+sub_tree_src]+pp_net_unit,
                                                                           (src_bus_msb,src_bus_lsb,src_bus_num),
                                                                           (dest_bus_msb,dest_bus_lsb,dest_bus_num)))

        nd['pp'+sub_tree_src]+=pp_net_unit
        nd['in_page_wires_used'][dest_page_num]+=dw_in_list[dest_page_num][dest_port_num]
        nd['out_page_wires_used'][src_page_num]+=dw_out_list[src_page_num][src_port_num]
        nd['in_page_ports_used'][int(dest_page_num)]+=1
        nd['out_page_ports_used'][int(src_page_num)]+=1
      else:
        # if source and dest are in the differen sub_trees
        # add routing path in src subtree, net_center and dest subtree

        # update src subtree
        src_bus_lsb = self.accumulate_wire_num(dw_out_list[src_page_num], src_port_num)
        src_bus_msb = src_bus_lsb+dw_out_list[src_page_num][src_port_num]-1
        src_bus_num = src_port_num
        dest_bus_lsb = nd['in_center_wires_used'][int(sub_tree_src)]
        dest_bus_msb = dest_bus_lsb+dw_in_list[dest_page_num][dest_port_num]-1
        dest_bus_num = nd['in_center_ports_used'][int(sub_tree_src)]

        if int(sub_tree_src) != 0:
          nd['net'+sub_tree_src].append('\n\n// ' + src + '=>' + dest)
          nd['net'+sub_tree_src].extend(self.return_inst_RelayStation_v_list(dw_out_list[src_page_num][src_port_num],
                                                                           'page'+str(src_page_num),
                                                                           'net'+str(sub_tree_src),
                                                                           nd['pp'+sub_tree_src],
                                                                           nd['pp'+sub_tree_src]+pp_net_unit,
                                                                           (src_bus_msb,src_bus_lsb,src_bus_num),
                                                                           (dest_bus_msb,dest_bus_lsb,dest_bus_num)))

        # update center net

        nd['net0'].append('\n\n// ' + src + '=>' + dest)
        src_bus_lsb = nd['in_center_wires_used'][int(sub_tree_src)] if int(sub_tree_src) != 0 else self.accumulate_wire_num(dw_out_list[src_page_num], src_port_num)
        src_bus_msb = src_bus_lsb+dw_out_list[src_page_num][src_port_num]-1 if int(sub_tree_src) != 0 else src_bus_lsb+dw_out_list[src_page_num][src_port_num]-1
        src_bus_num = nd['in_center_ports_used'][int(sub_tree_src)] if int(sub_tree_src) != 0 else src_port_num
        dest_bus_lsb = nd['out_center_wires_used'][int(sub_tree_dest)] if int(sub_tree_dest) != 0 else  self.accumulate_wire_num(dw_in_list[dest_page_num], dest_port_num)
        dest_bus_msb = dest_bus_lsb+dw_in_list[dest_page_num][dest_port_num]-1 if int(sub_tree_dest) != 0 else dest_bus_lsb+dw_in_list[dest_page_num][dest_port_num]-1
        dest_bus_num = nd['out_center_ports_used'][int(sub_tree_dest)] if int(sub_tree_dest) != 0 else dest_port_num

        nd['net0'].extend(self.return_inst_RelayStation_v_list(dw_out_list[src_page_num][src_port_num],
                                                               'net'+str(sub_tree_src) if int(sub_tree_src) != 0 else 'page'+str(src_page_num),
                                                               'net'+str(sub_tree_dest) if int(sub_tree_dest) != 0 else 'page'+str(dest_page_num),
                                                               nd['pp0'],
                                                               nd['pp0']+pp_center_unit,
                                                               (src_bus_msb,src_bus_lsb,src_bus_num),
                                                               (dest_bus_msb,dest_bus_lsb,dest_bus_num)))

        # update dest subtree
        src_bus_lsb = nd['out_center_wires_used'][int(sub_tree_dest)]
        src_bus_msb = src_bus_lsb+dw_out_list[src_page_num][src_port_num]-1
        src_bus_num = nd['out_center_ports_used'][int(sub_tree_dest)]
        dest_bus_lsb = self.accumulate_wire_num(dw_in_list[dest_page_num], dest_port_num)
        dest_bus_msb = dest_bus_lsb+dw_in_list[dest_page_num][dest_port_num]-1
        dest_bus_num = dest_port_num

        if int(sub_tree_dest) != 0:
          nd['net'+sub_tree_dest].append('\n\n// ' + src + '=>' + dest)
          nd['net'+sub_tree_dest].extend(self.return_inst_RelayStation_v_list(dw_out_list[src_page_num][src_port_num],
                                                                           'net'+str(sub_tree_dest),
                                                                           'page'+str(dest_page_num),
                                                                           nd['pp'+sub_tree_dest],
                                                                           nd['pp'+sub_tree_dest]+pp_net_unit,
                                                                           (src_bus_msb,src_bus_lsb,src_bus_num),
                                                                           (dest_bus_msb,dest_bus_lsb,dest_bus_num)))


        nd['pp0']+=pp_center_unit
        if int(sub_tree_src) != 0:
          nd['in_center_wires_used'][int(sub_tree_src)]+=dw_out_list[src_page_num][src_port_num]
          nd['in_center_ports_used'][int(sub_tree_src)]+=1
          nd['pp'+sub_tree_src]+=pp_net_unit
        if int(sub_tree_dest) != 0:
          nd['out_center_wires_used'][int(sub_tree_dest)]+=dw_in_list[dest_page_num][dest_port_num]
          nd['out_center_ports_used'][int(sub_tree_dest)]+=1
          nd['pp'+sub_tree_dest]+=pp_net_unit

        nd['in_page_wires_used'][dest_page_num]+=dw_in_list[dest_page_num][dest_port_num]
        nd['out_page_wires_used'][src_page_num]+=dw_out_list[src_page_num][src_port_num]
        nd['in_page_ports_used'][int(dest_page_num)]+=1
        nd['out_page_ports_used'][int(src_page_num)]+=1
    self.assign_dummy_driver(nd)
    for i in range(6): nd['net'+str(i)].append('endmodule')
    
    self.print_csv('/scratch/safe/ylxiao/page.csv',
           nd['in_page_wires_used'],
           nd['out_page_wires_used'],
           nd['in_page_ports_used'],
           nd['out_page_ports_used'])

    self.print_csv('/scratch/safe/ylxiao/center.csv',
           nd['in_center_wires_used'],
           nd['out_center_wires_used'],
           nd['in_center_ports_used'],
           nd['out_center_ports_used'])


    return nd


  def print_csv(self, file_name, list1, list2, list3, list4):
    file_in = open(file_name, 'w')

    for i in range(len(list1)):
      file_in.write(str(i)+','+str(list1[i])+','+str(list2[i])+','+str(list3[i])+','+str(list4[i])+'\n')
    
    file_in.close()
      




  def assign_dummy_driver(self, nd):
    for i in range(2**int(self.prflow_params['addr_bits'])):
      ports_used = nd['out_page_ports_used'][i]
      ports_avl = int(self.prflow_params['page'+str(i)+'_net'][1])
      if ports_used < ports_avl:
        nd['net'+str(self.net_list[i])].append('assign page'+str(i)+'_ready_upward['+str(ports_avl-1)+':'+str(ports_used)+']=0;')

      ports_used = nd['in_page_ports_used'][i]
      ports_avl = int(self.prflow_params['page'+str(i)+'_net'][1])
      if ports_used < ports_avl:
        nd['net'+str(self.net_list[i])].append('assign page'+str(i)+'_val_out['+str(ports_avl-1)+':'+str(ports_used)+']=0;')

      wires_used = nd['in_page_wires_used'][i]
      width_avl = int(self.prflow_params['page'+str(i)+'_net'][0])
      ports_avl = int(self.prflow_params['page'+str(i)+'_net'][1])
      wires_avl = width_avl*ports_avl
      if wires_used < wires_avl:
        nd['net'+str(self.net_list[i])].append('assign page'+str(i)+'_dout['+str(wires_avl-1)+':'+str(wires_used)+']=0;')

    for i in range(1, 6):
      # top tree: assign unsued ready to 0
      # sub tree: assign unsued valid to 0
      ports_used = nd['in_center_ports_used'][i]
      ports_avl = int(self.prflow_params['net0_net'+str(i)][1])
      if ports_used < ports_avl:
        nd['net0'].append('assign net'+str(i)+'_ready_upward['+str(ports_avl-1)+':'+str(ports_used)+']=0;')
        nd['net'+str(i)].append('assign net'+str(i)+'_val_out['+str(ports_avl-1)+':'+str(ports_used)+']=0;')

      # top tree: assign unsued dout to 0
      wires_used = nd['out_center_wires_used'][i]
      width_avl = int(self.prflow_params['net0_net'+str(i)][0])
      ports_avl = int(self.prflow_params['net0_net'+str(i)][1])
      wires_avl = width_avl*ports_avl
      if wires_used < wires_avl:
        nd['net0'].append('assign net'+str(i)+'_dout['+str(wires_avl-1)+':'+str(wires_used)+']=0;')

      # top tree: assign unsued valid to 0
      # sub tree: assign unsued ready to 0
      ports_used = nd['out_center_ports_used'][i]
      ports_avl = int(self.prflow_params['net0_net'+str(i)][1])
      if ports_used < ports_avl:
        nd['net0'].append('assign net'+str(i)+'_val_out['+str(ports_avl-1)+':'+str(ports_used)+']=0;')
        nd['net'+str(i)].append('assign net'+str(i)+'_ready_upward['+str(ports_avl-1)+':'+str(ports_used)+']=0;')

      # sub tree: assign unsued dout to 0
      wires_used = nd['in_center_wires_used'][i]
      width_avl = int(self.prflow_params['net0_net'+str(i)][0])
      ports_avl = int(self.prflow_params['net0_net'+str(i)][1])
      wires_avl = width_avl*ports_avl
      if wires_used < wires_avl:
        nd['net'+str(i)].append('assign net'+str(i)+'_dout['+str(wires_avl-1)+':'+str(wires_used)+']=0;')

  # verilog functions end
  ######################################################################################################################################################




  ######################################################################################################################################################
  # tcl function start
  def return_delete_bd_objs_tcl_str(self, obj_name):
    return('delete_bd_objs  [get_bd_cells '+obj_name+']')

  def return_change_parameter_tcl_str(self, cell_name, par_name, par_value):
    return('set_property -dict [list CONFIG.'+par_name+' {'+str(par_value)+'}] [get_bd_cells '+cell_name+']')

  def return_connect_bd_net_tcl_str(self, src_pin, dest_pin):
    return('connect_bd_net [get_bd_pins '+src_pin + '] [get_bd_pins '+dest_pin+']')

  def return_create_bd_cell_tcl_str(self, obj_name, inst_name, obj_type='ip'):
    return('create_bd_cell -type '+obj_type+' -vlnv '+obj_name+' ' + inst_name)

  def return_connect_bd_stream_tcl_list(self, src_name, dest_name, src_port_name='', dest_port_name=''):
    return([
      'connect_bd_net [get_bd_pins /'+src_name+'/'+src_port_name+'dout] [get_bd_pins /'+dest_name+'/'+dest_port_name+'din]',
      'connect_bd_net [get_bd_pins /'+src_name+'/'+src_port_name+'val_out] [get_bd_pins /'+dest_name+'/'+dest_port_name+'val_in]',
      'connect_bd_net [get_bd_pins /'+src_name+'/'+src_port_name+'ready_upward] [get_bd_pins /'+dest_name+'/'+dest_port_name+'ready_downward]',
      ''])

  def return_syn2bits_tcl_list(self, prj_dir='./prj/', prj_name = 'floorplan_static'):
    return ([
      'open_project '+prj_dir+prj_name+'.xpr',
      'reset_run synth_1',
      'launch_runs synth_1 -jobs 8',
      'wait_on_run synth_1',
      'launch_runs impl_1 -to_step write_bitstream  -jobs 8',
      'wait_on_run impl_1',
      'file mkdir '+prj_dir+prj_name+'.sdk',
      'file copy -force '+prj_dir+prj_name\
        +'.runs/impl_1/'+prj_name+'_wrapper.sysdef '+prj_dir+prj_name+'.sdk/'+prj_name+'_wrapper.hdf ',
      ''])

  def return_syn2dcp_tcl_list(self, prj_dir='./prj/', prj_name = 'floorplan_static.xpr'):
    return ([
      'open_project '+prj_dir+prj_name,
      'reset_run synth_1',
      'launch_runs synth_1 -jobs 8',
      'wait_on_run synth_1',
      'open_run synth_1 -name synth_1',
      'write_checkpoint -force floorplan_static.dcp',
      ''])


  def return_qsub_scan_sh_list(self, scan_dir, run_shell='qsub_run.sh', hold_prefix='hls_', name_prefix='mono_bft_'):
    return ([
      '#!/bin/bash -e',
      #'module load ' + self.prflow_params['qsub_Xilinx_dir'],
      #'emailAddr="' + self.prflow_params['email'] + '"',
      'file_list=\'dummy\'',
      'for file in $(ls '+scan_dir+')',
      'do',
      '  if [ "$file" != "synth_1" ]; then',
      '    file_list=$file_list\',mono_bft_\'$file',
      '    cd \''+scan_dir+'/\'$file',
      #'    qsub    -hold_jid '+hold_prefix+'$file                           -N '+name_prefix+'$file   -q ' + self.prflow_params['qsub_grid'] + ' -m abe -M $emailAddr -l mem=8G  -cwd ./'+run_shell,
      '    sbatch --ntasks=1 --cpus-per-task=8 --mem-per-cpu=7500mb --job-name='+name_prefix+'$file  --dependency=$(squeue --noheader --format %i --user=$USER --name dummy | sed -n -e \'H;${x;s/\\n/,/g;s/^,//;p;}\') '+run_shell,
      '    cd -',
      '  fi',
      'done',
      ''])

  def return_ip_page_tcl_list(self, fun_name, fun_num):
    return ([
      'add_files -norecurse ../../../F001_static_' + self.prflow_params['nl'] + '_leaves/src/stream_shell.v',
      'add_files -norecurse ../../../F001_static_' + self.prflow_params['nl'] + '_leaves/src/RelayStation.v',
      'set dir "../../../F002_hls_'+self.prflow_params['benchmark_name']  + '/' + fun_name + '_prj/' + fun_name + '/syn/verilog"',
      'set contents [glob -nocomplain -directory $dir *]',
      'foreach item $contents {',
      '  if { [regexp {.*\.tcl} $item] } {',
      '    source $item',
      '  } else {',
      '    add_files -norecurse $item',
      '  }',
      '}',
      'set_param general.maxThreads  8',
      'set_property XPM_LIBRARIES {XPM_CDC XPM_MEMORY XPM_FIFO} [current_project]',
      'add_files  -norecurse ./pe_empty_'+str(fun_num)+'.v',
      'ipx::package_project -root_dir ./prj/floorplan_static.srcs/sources_1 -vendor user.org -library user -taxonomy /UserIP',
      'set_property core_revision 2 [ipx::current_core]',
      'ipx::create_xgui_files [ipx::current_core]',
      'ipx::update_checksums [ipx::current_core]',
      'ipx::save_core [ipx::current_core]',
      'set_property  ip_repo_paths  ./prj/floorplan_static.srcs/sources_1 [current_project]',
      'update_ip_catalog',
      ''])

  def return_dummy_logic_tcl_list(self, fun_name):
    return ([
      'set_param general.maxThreads  8',
      'add_files  -norecurse ./'+fun_name+'.v',
      'set_property XPM_LIBRARIES {XPM_CDC XPM_MEMORY XPM_FIFO} [current_project]',
      'set logFileId [open ./runLog_'+fun_name+'.log "a"]',
      'set start_time [clock seconds]',
      'set_param general.maxThreads  8 ',
      'synth_design -top '+fun_name+' -part '+self.prflow_params['part']+' -mode out_of_context',
      'write_checkpoint -force netlist.dcp',
      'set end_time [clock seconds]',
      'set total_seconds [expr $end_time - $start_time]',
      'puts $logFileId "syn: $total_seconds seconds"',
      'report_utilization -hierarchical > utilization.rpt',
      ''])

  def return_syn_page_tcl_list(self, fun_name, page_name):
    return ([
      #'add_files -norecurse ../../F001_static_' + self.prflow_params['nl'] + '_leaves/src/stream_cnt.v',
      'add_files -norecurse ../../F001_static_'+self.prflow_params['nl']+'_leaves/src/stream_shell.v',
      'add_files -norecurse ../../F001_static_' + self.prflow_params['nl'] + '_leaves/src/RelayStation.v',
      'set dir "../../F002_hls_'+self.prflow_params['benchmark_name']  + '/' + fun_name + '_prj/' + fun_name + '/syn/verilog"',
      'set contents [glob -nocomplain -directory $dir *]',
      'foreach item $contents {',
      '  if { [regexp {.*\.tcl} $item] } {',
      '    source $item',
      '  } else {',
      '    add_files -norecurse $item',
      '  }',
      '}',
      'set_param general.maxThreads  8',
      'add_files  -norecurse ./'+page_name+'.v',
      'set_property XPM_LIBRARIES {XPM_CDC XPM_MEMORY XPM_FIFO} [current_project]',
      'set logFileId [open ./runLog_'+fun_name+'.log "a"]',
      'set start_time [clock seconds]',
      'set_param general.maxThreads  8 ',
      'synth_design -top '+page_name+' -part '+self.prflow_params['part']+' -mode out_of_context',
      'write_checkpoint -force page_netlist.dcp',
      'set end_time [clock seconds]',
      'set total_seconds [expr $end_time - $start_time]',
      'puts $logFileId "syn: $total_seconds seconds"',
      'report_utilization -hierarchical > utilization.rpt',
      ''])

  def return_syn_net_tcl_list(self, net_name):
    return ([
      'add_files -norecurse ../../F001_static_'+self.prflow_params['nl']+'_leaves/src/RelayStation.v',
      'set_param general.maxThreads  8',
      'add_files  -norecurse ./'+net_name+'.v',
      'set_property XPM_LIBRARIES {XPM_CDC XPM_MEMORY XPM_FIFO} [current_project]',
      'set logFileId [open ./runLog_'+net_name+'.log "a"]',
      'set start_time [clock seconds]',
      'set_param general.maxThreads  8 ',
      'synth_design -top '+net_name+' -part '+self.prflow_params['part']+' -mode out_of_context',
      'write_checkpoint -force '+net_name+'_netlist.dcp',
      'set end_time [clock seconds]',
      'set total_seconds [expr $end_time - $start_time]',
      'puts $logFileId "syn: $total_seconds seconds"',
      'report_utilization -hierarchical > utilization.rpt',
      ''])

  def return_impl_tcl_list(self, fun_name, num, IsNet=False):
    lines_list = []
    lines_list.append('set logFileId [open ./runLogImpl_'+fun_name+'.log "w"]')
    #lines_list.append('set_param general.maxThreads ' + self.prflow_params['maxThreads'] + ' ')
    lines_list.append('set_param general.maxThreads 2 ')
    lines_list.append('')
    lines_list.append('#####################')
    lines_list.append('## read_checkpoint ##')
    lines_list.append('#####################')
    lines_list.append('set start_time [clock seconds]')
    lines_list.append('open_checkpoint ../../F001_static_' + self.prflow_params['nl'] + '_leaves/big_static_routed_' + self.prflow_params['nl'] + '.dcp')
    #lines_list.append('open_checkpoint ../../F001_static_' + self.prflow_params['nl'] + '_leaves/init_routed_' + self.prflow_params['nl'] + '.dcp')
    if IsNet:
      lines_list.append("update_design -cell floorplan_static_i/net" + str(num) + "/inst -black_box")
      lines_list.append("read_checkpoint -cell floorplan_static_i/net" + str(num) + "/inst ../../F003_syn_" + self.prflow_params['benchmark_name'] + '/net' + str(num) + "/net" + str(num) + "_netlist.dcp")
    else:
      lines_list.append("update_design -cell floorplan_static_i/pe_empty_" + str(num) + "/inst -black_box")
      lines_list.append("read_checkpoint -cell floorplan_static_i/pe_empty_" + str(num) + "/inst ../../F003_syn_" + self.prflow_params['benchmark_name'] + '/' + fun_name + "/page_netlist.dcp")
 
    #lines_list.append('add_files -norecurse ../../F001_static_' + self.prflow_params['nl'] + '_leaves/src/clock_constraints.xdc')
    lines_list.append("set end_time [clock seconds]")
    lines_list.append("set total_seconds [expr $end_time - $start_time]")
    lines_list.append('puts $logFileId "read_checkpoint: $total_seconds seconds"')
    lines_list.append("")
    lines_list.append("")

    lines_list.append("####################")
    lines_list.append("## implementation ##")
    lines_list.append("####################")
    lines_list.append("set start_time [clock seconds]")
    lines_list.append("#reset_timing ")
    lines_list.append("opt_design ")
    lines_list.append("set end_time [clock seconds]")
    lines_list.append("set total_seconds [expr $end_time - $start_time]")
    lines_list.append('puts $logFileId "opt: $total_seconds seconds"')
    lines_list.append("write_checkpoint  -force  "+ fun_name + "_opt.dcp")
    lines_list.append("")

    lines_list.append("set start_time [clock seconds]")
    if self.prflow_params['PR_mode'] == 'quick':
      lines_list.append("place_design -directive Quick ")
    else:
      lines_list.append("place_design  ")

    lines_list.append("set end_time [clock seconds]")
    lines_list.append("set total_seconds [expr $end_time - $start_time]")
    lines_list.append('puts $logFileId "place: $total_seconds seconds"')
    lines_list.append("write_checkpoint  -force  "+fun_name + "_placed.dcp")
    lines_list.append("")

    lines_list.append("set start_time [clock seconds]")
    if self.prflow_params['PR_mode'] == 'quick':
      lines_list.append("route_design -directive Quick ")
    else:
      lines_list.append("route_design  ")
          
    lines_list.append("set end_time [clock seconds]")
    lines_list.append("set total_seconds [expr $end_time - $start_time]")
    lines_list.append('puts $logFileId "route: $total_seconds seconds"')
    lines_list.append("write_checkpoint -force   " + fun_name + "_routed.dcp")
    lines_list.append('report_timing_summary > ./timing.rpt')     
    lines_list.append("")
    lines_list.append("")
    
    lines_list.append("###############")
    lines_list.append("## bitstream ##")
    lines_list.append("###############")
    lines_list.append("set_param bitstream.enablePR 2341")
    lines_list.append("set start_time [clock seconds]")
    if IsNet:
      lines_list.append("write_bitstream  -force  -cell floorplan_static_i/net" + str(num) + "/inst ../../F005_bits_"+self.prflow_params['benchmark_name']+"/net_" + str(num) + "")
    else:
      lines_list.append("write_bitstream  -force  -cell floorplan_static_i/pe_empty_" + str(num) + "/inst ../../F005_bits_"+self.prflow_params['benchmark_name']+"/leaf_" + str(num) + "")
    lines_list.append("set end_time [clock seconds]")
    lines_list.append("set total_seconds [expr $end_time - $start_time]")
    lines_list.append('puts $logFileId "bit_gen: $total_seconds seconds"')
    if IsNet:
      lines_list.append('report_utilization -pblocks ' + fun_name + ' > utilization.rpt')
    else:
      lines_list.append('report_utilization -pblocks p_' + str(num) + ' > utilization.rpt')
    lines_list.append('report_timing_summary > timing.rpt')
    return lines_list






  def return_mk_overlay_tcl_list(self):
    lines_list = []
    lines_list.append('set logFileId [open ./runLog_impl_big_static_' + str(self.prflow_params['nl']) + '.log "w"]')
    lines_list.append('')
    lines_list.append('#####################')
    lines_list.append('## read_checkpoint ##')
    lines_list.append('#####################')
    lines_list.append('set start_time [clock seconds]')
    lines_list.append('open_checkpoint ./floorplan_static.dcp')
    for i in range(2,4):
      for j in range(3):
        lines_list.append('read_checkpoint -cell floorplan_static_i/pe_empty_X' + str(i) + 'Y' + str(j) + '/inst ./dummy_repo/X' + str(i) + 'Y' + str(j) + '/netlist.dcp')
    for i in range(4):
      for j in range(3,7):
        lines_list.append('read_checkpoint -cell floorplan_static_i/pe_empty_X' + str(i) + 'Y' + str(j) + '/inst ./dummy_repo/X' + str(i) + 'Y' + str(j) + '/netlist.dcp')
    
    lines_list.append('set end_time [clock seconds]')
    lines_list.append('set total_seconds [expr $end_time - $start_time]')
    lines_list.append('puts $logFileId "read_checkpoint: $total_seconds seconds"')
    lines_list.append('')
    lines_list.append('####################')
    lines_list.append('## implementation ##')
    lines_list.append('####################')
    lines_list.append('set start_time [clock seconds]')
    lines_list.append('opt_design')
    lines_list.append('set end_time [clock seconds]')
    lines_list.append('set total_seconds [expr $end_time - $start_time]')
    lines_list.append('puts $logFileId "opt: $total_seconds seconds"')
    lines_list.append('set start_time [clock seconds]')
    lines_list.append('place_design')
    lines_list.append('set end_time [clock seconds]')
    lines_list.append('set total_seconds [expr $end_time - $start_time]')
    lines_list.append('puts $logFileId "place: $total_seconds seconds"')
    lines_list.append('# write_hwdef -force pr_test_wrapper.hwdef')
    lines_list.append('write_checkpoint -force init_placed_' + str(self.prflow_params['nl']) + '.dcp')
    lines_list.append('set start_time [clock seconds]')
    lines_list.append('route_design')
    lines_list.append('set end_time [clock seconds]')
    lines_list.append('set total_seconds [expr $end_time - $start_time]')
    lines_list.append('puts $logFileId "route: $total_seconds seconds"')
    lines_list.append('write_checkpoint -force init_routed_' + str(self.prflow_params['nl']) + '.dcp')
    lines_list.append('set_param bitstream.enablePR 2341')
    lines_list.append('write_bitstream -force -no_partial_bitfile  ./main.bit')
    lines_list.append('#############################################')
    lines_list.append('## create static design with no bft pblock ##')
    lines_list.append('#############################################')
    lines_list.append('')
    lines_list.append('set start_time [clock seconds]')
    lines_list.append('update_design -cell floorplan_static_i/axi_leaf -black_box')
    for i in range(2,4):
      for j in range(3):
        lines_list.append('update_design -cell floorplan_static_i/pe_empty_X' + str(i) + 'Y' + str(j) + '/inst -black_box')
    for i in range(4):
      for j in range(3,7):
        lines_list.append('update_design -cell floorplan_static_i/pe_empty_X' + str(i) + 'Y' + str(j) + '/inst -black_box')
 

    lines_list.append('#############################################')
    lines_list.append('## Only after empty all modules out, can   ##')
    lines_list.append('## you add -buffer_ports                   ##')
    lines_list.append('#############################################')
 
    lines_list.append('update_design -cell floorplan_static_i/axi_leaf -buffer_ports')
    for i in range(2,4):
      for j in range(3):
        lines_list.append('update_design -cell floorplan_static_i/pe_empty_X' + str(i) + 'Y' + str(j) + '/inst -buffer_ports')
    for i in range(4):
      for j in range(3,7):
        lines_list.append('update_design -cell floorplan_static_i/pe_empty_X' + str(i) + 'Y' + str(j) + '/inst -buffer_ports')
 

    lines_list.append('lock_design -level routing')
    lines_list.append('write_checkpoint -force big_static_routed_' + str(self.prflow_params['nl']) + '.dcp')
    lines_list.append('close_design')
    lines_list.append('set end_time [clock seconds]')
    lines_list.append('set total_seconds [expr $end_time - $start_time]')
    lines_list.append('puts $logFileId "update, black_box: $total_seconds seconds"')
    lines_list.append('# set start_time [clock seconds]')
    lines_list.append('# set end_time [clock seconds]')
    lines_list.append('# set total_seconds [expr $end_time - $start_time]')
    lines_list.append('# puts $logFileId "write bitstream: $total_seconds seconds"')

    return lines_list

  # tcl command function end
  ######################################################################################################################################################


  ######################################################################################################################################################
  # help functions start
  def print_params(self):
    print self.prflow_params

  def print_list(self, in_list):
    for num, value in enumerate(in_list):
      print str(num)+'\t'+str(value)
    print ''

  def print_dict(self, in_dict):
    key_list = in_dict.keys()
    key_list.sort()
    for key in key_list:
      print str(key)+'->'+str(in_dict[key])

  def has_pattern(self, in_str, pattern_str):
    if in_str.replace(pattern_str, '') == in_str:
      return False
    else:
      return True

  # help functions end
  ######################################################################################################################################################

if __name__ == '__main__':
  modification_dict = {'parameter PAYLOAD_BITS0': 'parameter PAYLOAD_BITS0 = a,',
                       'parameter PORT_NUM_IN0': 'parameter PORT_NUM_IN0 = b,',
                       'parameter PORT_NUM_OUT0': 'parameter PORT_NUM_OUT0 = c,'}
  filename='net0.v'
  inst = gen_basic()
  inst.replace_lines(filename, modification_dict)
  lines_list = ['hell',
   'you',
   'are',
   'every',
   'thing']

  inst.add_lines(filename, 'input clk', lines_list)
  inst.write_lines('net1.txt',  lines_list)



