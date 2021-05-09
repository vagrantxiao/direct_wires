# -*- coding: utf-8 -*-   

import os  
import subprocess
from gen_basic import gen_basic


class gen_impl_leaf_files(gen_basic):

  def get_key(self, val, my_dict): 
    for key, value in my_dict.items(): 
      if val == value: 
        return key
    return 'switchbox'

  def get_entry(self, val, my_list):
    for i in range(len(my_list)): 
      if val == my_list[i]:
        return i 


  def get_all_used_pages(self):
    links = self.prflow_params['links'] 
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


  # create one directory for each page 
  def create_page(self):
    used_pages_list = self.get_all_used_pages()

    #for fun_num, fun_name in enumerate(self.prflow_params['syn_fun_list']):
    for page in used_pages_list:    
      page_num = page
      fun_name = self.get_key(page, self.prflow_params['mapping_dict'])
      num_bram_addr_bits =int(self.prflow_params['bram_addr_bits'])
      self.re_mkdir(self.pr_dir+'/page_'+str(page_num))
      self.write_lines(self.pr_dir+'/page_'+str(page_num)+'/impl_'+fun_name+'.tcl', self.return_impl_tcl_list(page, page_num, False))
      self.write_lines(self.pr_dir+'/page_'+str(page_num)+'/qsub_run.sh', self.return_run_sh_list(self.prflow_params['qsub_Xilinx_dir'], 'impl_'+fun_name+'.tcl'), True)


  # main.sh will be used for local compilation
  def return_qsub_main_sh_list_local(self):
    lines_list = []
    lines_list.append('#!/bin/bash -e')
    used_pages_list = self.get_all_used_pages()
    # compile the IP for each page
    for page in used_pages_list:    
      page_num = page
      lines_list.append('cd ./page_'+str(page_num)) 
      lines_list.append(self.return_qsub_command_str('./qsub_run.sh', 'syn_'+page, 'impl_'+page, self.prflow_params['MEM'], self.prflow_params['qsub_grid'], self.prflow_params['email'], self.prflow_params['qsub_Nodes']))
      lines_list.append('cd -') 
    return lines_list


  # main.sh will be used for local compilation
  def return_main_sh_list_local(self):
    lines_list = []
    lines_list.append('#!/bin/bash -e')
    lines_list.append('source '+self.prflow_params['Xilinx_dir'])

    # compile the IP for each page
    iter_num = 1
    used_pages_list = self.get_all_used_pages()
    for page in used_pages_list:    
      page_num = page
      fun_name = self.get_key(page, self.prflow_params['mapping_dict'])
      lines_list.append('cd ./page_'+str(page_num)) 
      if iter_num % int(self.prflow_params['jobNum']) == 0:
        lines_list.append('vivado -mode batch -source ./impl_'+fun_name+'.tcl') 
      elif iter_num == len(used_pages_list):
        lines_list.append('vivado -mode batch -source ./impl_'+fun_name+'.tcl') 
      else:
        lines_list.append('vivado -mode batch -source ./impl_'+fun_name+'.tcl&') 
      lines_list.append('cd -') 
      iter_num += 1
    return lines_list


  def create_shell_file(self):
  # local run:
  #   main.sh <- |_ execute each impl_page.tcl
  #
  # qsub run:
  #   qsub_main.sh <-|_ Qsubmit each qsub_run.sh <- impl_page.tcl
   
    self.write_lines(self.pr_dir+'/main.sh', self.return_main_sh_list_local(), True)
    self.write_lines(self.pr_dir+'/qsub_main.sh', self.return_qsub_main_sh_list_local(), True)



  def run(self):
    # mk work directory
    if self.prflow_params['leaf_impl_regen']=='1':
      self.re_mkdir(self.pr_dir)
    
    # generate shell files for qsub run and local run
    self.create_shell_file() 

    # create ip directories for all the pages
    self.create_page()





    # go to the local mono_bft directory and run the qsub command
    os.chdir(self.pr_dir)
    if self.prflow_params['run_qsub']:
      if self.prflow_params['gen_lib'] != True:
        os.system('./qsub_main.sh')
    os.chdir('../../')

   

