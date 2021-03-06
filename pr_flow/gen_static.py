# -*- coding: utf-8 -*-   

import os  
import subprocess
from gen_basic import gen_basic

class gen_static(gen_basic):
  def __init__(self, prflow_params):
    gen_basic.__init__(self, prflow_params)
    self.dummy_list = []
    for i in range(2,4):
      for j in range(3):
        self.dummy_list.append('X' + str(i) + 'Y' + str(j))
    for i in range(4):
      for j in range(3,7):
        self.dummy_list.append('X' + str(i) + 'Y' + str(j))
 
  # create dummy directory for each empty block
  def create_place_holder(self):
    self.re_mkdir(self.static_dir+'/dummy_repo')
    for fun_name in self.place_holder:
      self.re_mkdir(self.static_dir+'/dummy_repo/'+fun_name)
      self.write_lines(self.static_dir+'/dummy_repo/'+fun_name+'/dummy.tcl', self.return_dummy_logic_tcl_list(self.place_holder[fun_name]))
      self.write_lines(self.static_dir+'/dummy_repo/'+fun_name+'/run.sh',      self.return_run_sh_list(self.prflow_params['Xilinx_dir'], 'dummy.tcl'), True)
      self.write_lines(self.static_dir+'/dummy_repo/'+fun_name+'/qsub_run.sh', self.return_run_sh_list(self.prflow_params['qsub_Xilinx_dir'], 'dummy.tcl'), True)
      self.cp_dir(self.static_dir+'/src/place_holder/'+self.place_holder[fun_name]+'.v', self.static_dir+'/dummy_repo/'+fun_name)
      width_list = self.prflow_params['datawidth_dict'][fun_name]
      replace_dict = {
                      'parameter WEST_WIDTH': 'parameter WEST_WIDTH = '+str(width_list[0])+',',
                      'parameter NORTH_WIDTH': 'parameter NORTH_WIDTH = '+str(width_list[1])+',',
                      'parameter SOUTH_WIDTH': 'parameter SOUTH_WIDTH = '+str(width_list[2])+',',
                      'parameter EAST_WIDTH': 'parameter EAST_WIDTH = '+str(width_list[3])+','
                      }
      self.replace_lines(self.static_dir+'/dummy_repo/'+fun_name+'/'+self.place_holder[fun_name]+'.v',  replace_dict) 

  # main.sh will be used for local compilation
  def return_main_sh_list_local(self):
    lines_list = []
    lines_list.append('#!/bin/bash -e')
    lines_list.append('source '+self.prflow_params['Xilinx_dir'])
    print 'maxThreads: ' + self.prflow_params['maxThreads'] 
    # compile the dummy logic for each page
    iter_num = 1
    for fun_name in self.dummy_list:
      lines_list.append('cd ./dummy_repo/' + fun_name)
      if iter_num % int(self.prflow_params['jobNum']) != 0:
        lines_list.append('./run.sh&')
      else:
        lines_list.append('./run.sh')
      iter_num += 1
      lines_list.append('cd -')
    lines_list.append('vivado -mode batch -source project_syn2gen.tcl')
    lines_list.append('vivado -mode batch -source project_syn2dcp.tcl')
    lines_list.append('vivado -mode batch -source mk_overlay.tcl')
    return lines_list


  # qsub_main.sh will be used for qsub compilation  
  def return_qsub_main_sh_list_local(self):
    # go through all the files and qsub the ip compilation tasks
    lines_list = self.return_qsub_scan_sh_list('./dummy_repo', 'qsub_run.sh',  '', 'dummy_')
    hold_jid = 'dummy'

    # after the ip compilation is done, we can construct the vivado momo bft project
    lines_list.append(self.return_qsub_command_str('./qsub_project_syn2gen.sh', hold_jid, 'static_syn2gen'))

    # we can accelerate the synthesis by compile each out-of-context modules in parallel
    lines_list.append(self.return_qsub_command_str('./qsub_sub_syn.sh', 'static_syn2gen', 'static_sub_syn'))
    return lines_list


  # qsub_sub_syn.sh will go through all the out-of-context module directories and qsub each 
  # task by executing thn runme.sh, which is generated by vivado 
  def return_sub_syn_sh_list_local(self):
    lines_list = self.return_qsub_scan_sh_list('./prj/floorplan_static.runs', 'runme.sh', '', 'static_sub_')
    hold_jid = '$file_list'

    # after all the out-of-context compilations are done, we reopen the project and compile it to bits.
    lines_list.append(self.return_qsub_command_str('./qsub_project_syn2dcp.sh', hold_jid, 'syn2dcp'))
    lines_list.append(self.return_qsub_command_str('./qsub_mk_overlay.sh', 'syn2dcp', 'mk_overlay'))
    lines_list.append('cd ../..')
    lines_list.append('cd ' + self.pr_dir)
    lines_list.append(self.return_qsub_command_str('./qsub_main.sh', 'mk_overlay', 'submit_pr'))
    return lines_list

  def return_syn2gen_tcl_list_local(self):
    # change the page and network parameters accoring to xml files
    lines_list = []
    datawidth_dict = self.prflow_params['datawidth_dict'] 
    
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
 
    return lines_list 

  def create_shell_file(self):
  # local run:
  #   main.sh <- |_ vivado each dummy.tcl
  #              |_ vivado project_syn2gen.tcl
  #              |_ vivado project_syn2dcp.tcl
  #              |_ vivado mk_overlay.tcl
  #
  # qsub run:
  #   qsub_main.sh <-|_ qsubmit each qsub_run.sh <- dummy.tcl
  #                  |_ qsub_project_syn2gen.sh <- project_syn2gen.tcl  
  #                  |_ qsub_sub_syn.sh <-|_ go through very synthesis directies and Qsubmmit job
  #                                       |_ qsub_project_syn2dcp.sh <- project_syn2dcp.tcl
  #                                       |_ qsub_mk_overlay.sh <- mk_overlay.tcl
  #                                       |_ go to F004 ./qsub_main.sh

    #generate tcl file to create the static region vivado project
    #we use existed files instead
    self.cp_dir('./input_files/script_src/project_syn_gen_'+self.prflow_params['board']+'.tcl ', self.static_dir +'/project_syn2gen.tcl')
    self.add_lines(self.static_dir+'/project_syn2gen.tcl', '# Create address segments', self.return_syn2gen_tcl_list_local())
    self.write_lines(self.static_dir+'/project_syn2dcp.tcl', self.return_syn2dcp_tcl_list(), False)
    self.write_lines(self.static_dir+'/mk_overlay.tcl', self.return_mk_overlay_tcl_list(), False)

    self.write_lines(self.static_dir+'/main.sh', self.return_main_sh_list_local(), True)
    self.write_lines(self.static_dir+'/qsub_main.sh', self.return_qsub_main_sh_list_local(), True)
    self.write_lines(self.static_dir+'/qsub_project_syn2gen.sh', self.return_run_sh_list(self.prflow_params['qsub_Xilinx_dir'], 'project_syn2gen.tcl'), True)
    self.write_lines(self.static_dir+'/qsub_sub_syn.sh', self.return_sub_syn_sh_list_local(), True)
    self.write_lines(self.static_dir+'/qsub_project_syn2dcp.sh', self.return_run_sh_list(self.prflow_params['qsub_Xilinx_dir'], 'project_syn2dcp.tcl'), True)
    self.write_lines(self.static_dir+'/qsub_mk_overlay.sh', self.return_run_sh_list(self.prflow_params['qsub_Xilinx_dir'], 'mk_overlay.tcl'), True)



  def run(self):
    # make work directory
    if int(self.prflow_params['static_regen']) == 1:
      self.re_mkdir(self.static_dir)
    
    # copy the hld/xdc files from input source directory
    self.cp_dir('./input_files/interface_src', self.static_dir+'/src')

    # generate shell files for qsub run and local run
    self.create_shell_file()

    self.create_place_holder()

    if self.prflow_params['run_qsub']:
        os.chdir(self.static_dir)
        os.system('./qsub_main.sh')
        os.chdir('../../')




