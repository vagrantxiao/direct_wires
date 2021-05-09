#!/usr/bin/env python
import sys
import os
import xml.etree.ElementTree
import argparse
import re
import math

def get_key(val, my_dict): 
  for key, value in my_dict.items(): 
    if val == value: 
      return key
  return 'switchbox'


def get_all_used_pages(links, mapping_dict):
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





if __name__ == "__main__":

  os.system('mkdir -p ./workspace/report')
  parser = argparse.ArgumentParser()
  parser.add_argument('benchmark_name')

  prflow_params = {} 
  args = parser.parse_args()
  benchmark_name = args.benchmark_name  
  input_file_name = './input_files/hls_src/' + benchmark_name + '/architecture.xml'
  root = xml.etree.ElementTree.parse(input_file_name).getroot()
  links = root.findall('link')	
  specs = root.findall('spec')
  network = root.findall('network')
  functions = root.findall('function')
  clock =root.findall('clock')
  datawidth_in=root.findall('datawidth_in')
  datawidth_out=root.findall('datawidth_out')
  place_holder=root.findall('place_holder')
  network=root.findall('network')


  place_holder_dict = {}
  hls_fun_list = []
  syn_fun_list = []
  input_num_list = []
  output_num_list = []
  page_list = []
  ram_type_list = ['block']
  mapping_dict = {'DMA': 'X1Y2'}

  for child in place_holder:
    place_holder_dict[child.get('pe')] = child.get('type')




  for child in functions:
    hls_fun_list.append(child.get('name'))
    syn_fun_list.append(child.get('name'))
    input_num_list.append(child.get('inputs'))
    output_num_list.append(child.get('outputs'))
    page_list.append(child.get('page'))
    ram_type_list.append(child.get('ramtype'))
    mapping_dict[child.get('name')] = child.get('page')

  for child in specs:
    prflow_params[child.get('name')] = child.get('value')
    
  for child in clock:
    prflow_params[child.get('name')] = child.get('period')

  if prflow_params['min_net'] == '1':  
    datawidth_dict = gen_min_width(links, mapping_dict)
  else: 
    datawidth_dict = {}
    for child in network:
      datawidth_dict[child.get('pe')] = [int(child.get('W')), int(child.get('N')), int(child.get('S')), int(child.get('E'))] 



 
  prflow_params['mapping_dict'] = mapping_dict
  prflow_params['hls_fun_list'] = hls_fun_list
  prflow_params['syn_fun_list'] = syn_fun_list
  prflow_params['input_num_list'] = input_num_list
  prflow_params['output_num_list'] = output_num_list
  prflow_params['page_list'] = page_list
  prflow_params['ram_type_list'] = ram_type_list
  prflow_params['datawidth_dict'] = datawidth_dict
  prflow_params['links'] = links
  prflow_params['place_holder_dict'] = place_holder_dict





 
  resource_report_file = open('./workspace/report/resource_report_'+benchmark_name+'.csv', 'w')
  time_report_file = open('./workspace/report/time_report_'+benchmark_name+'.csv', 'w')

  links = root.findall('link')	
  used_pages_list = get_all_used_pages(links, mapping_dict)
#####################################################################################
#process hls timing
  hls_mat = []
  for page in used_pages_list:
    fun_name = get_key(page, mapping_dict)
    try:
      if fun_name == 'switchbox':
        time_report_file.write('hls\t' + page + '\t0\n')
      else:
        file_name = './workspace/F002_hls_'+benchmark_name+'/runLog' + fun_name + '.log'
        file_in = open(file_name, 'r')
        for line in file_in:
          run_time = re.findall(r"\d+", line)
          time_report_file.write('hls\t' + fun_name + '\t' + run_time[0] + '\n')
          hls_mat.append(int(run_time[0]))
        file_in.close()
    except:
      print ('No '+file_name) 


#####################################################################################
#process syn timing for pages
  time_report_file.write('\n------------------------------------------------------------------------------' + '\n')
  syn_mat = []
  for page in used_pages_list:
    fun_name = get_key(page, mapping_dict)
    if fun_name != 'user_kernel': 
      try:
        file_name = './workspace/F003_syn_'+benchmark_name+'/' + page + '/runLog_' + fun_name + '.log'
        file_in = open(file_name, 'r')
        for line in file_in:
          run_time = re.findall(r"\d+", line)
          if fun_name == 'switchbox':
            time_report_file.write('syn\t' + fun_name + '\t' + run_time[0] + '\n')
          else:
            time_report_file.write('syn\t' + page + '\t' + run_time[0] + '\n')
        file_in.close()
      except:
        print ('No '+file_name) 
#

  time_report_file.write('\n------------------------------------------------------------------------------' + '\n')
  fun_entry = 0;


  LUTs_list = []
  FFs_list = []
  BRAMs_list = []
  DSPs_list = []
  
  rdchk_mat = []
  opt_mat = []
  place_mat = []
  route_mat = []
  bitgen_mat = []
  for page in used_pages_list:
      fun_name = get_key(page, mapping_dict)
      page_num = page
      #####################################################################################
      #process resource utilization
      try:
        file_name = './workspace/F003_syn_'+benchmark_name+'/' + page + '/utilization.rpt'
        file_in = open(file_name, 'r')
        for line in file_in:
          if line.startswith('| pe_empty'):
            luts =  re.findall(r"\d+", line)
            #str_tmp = fun_name + '\tpage_' + page_num + '\t' + luts[3] + ' / ' + luts[5]
            str_tmp = fun_name + '\tpage_' + page_num + '\t' + luts[1]
            LUTs_list.append(str_tmp)
  
          if line.startswith('| pe_empty'):
            FFs =  re.findall(r"\d+", line)
            #str_tmp = fun_name + '\tpage_' + page_num + '\t' + FFs[3] + ' / ' + FFs[5]
            str_tmp = fun_name + '\tpage_' + page_num + '\t' + FFs[5]
            FFs_list.append(str_tmp)
  
          if line.startswith('| pe_empty'):
            brams =  re.findall(r"\d+", line)
            #str_tmp = fun_name + '\tpage_' + page_num + '\t' + brams[3] + ' / ' + brams[5]
            str_tmp = fun_name + '\tpage_' + page_num + '\t' + str(int(brams[6])*2+int(brams[7]))
            BRAMs_list.append(str_tmp)
  
          if line.startswith('| pe_empty'):
            dsps =  re.findall(r"\d+", line)
            #str_tmp = fun_name + '\tpage_' + page_num + '\t' + dsps[3] + ' / ' + dsps[5]
            str_tmp = fun_name + '\tpage_' + page_num + '\t' + dsps[9]
            DSPs_list.append(str_tmp)
        file_in.close()
      except:
        print ('No '+file_name) 


  fun_entry = 0
  for page in used_pages_list:
      fun_name = get_key(page, mapping_dict)
      page_num = page
      #####################################################################################
      #process impl timing
      file_name = './workspace/F004_pr_'+benchmark_name+'/page_' + page_num + '/runLogImpl_' + page_num + '.log'
      try: 
        file_in = open(file_name, 'r')
        run_time_list = []
        for line in file_in:
          run_time = re.findall(r"\d+", line)
          run_time_list.append(run_time[0]) 
        file_in.close()
        str_tmp = fun_name + '->\tpage_' + page_num + '\t'
        for i in run_time_list:
          str_tmp = str_tmp + i + '\t'
        time_report_file.write(str_tmp + '\n')
        if len(run_time_list) > 2:
          rdchk_mat.append(int(run_time_list[0]))
          opt_mat.append(int(run_time_list[1]))
          place_mat.append(int(run_time_list[2]))
          route_mat.append(int(run_time_list[3]))
          #bitgen_mat.append(int(run_time_list[4]))
        file_in.close()
      except:
        print "No "+file_name



#####################################################################################

  resource_report_file.write('LUTs-----------------------------------------\n')
  for ele in LUTs_list:
    resource_report_file.write(ele + '\n')
  resource_report_file.write('FFs-----------------------------------------\n')
  for ele in FFs_list:
    resource_report_file.write(ele + '\n')
  resource_report_file.write('BRAMs-----------------------------------------\n')
  for ele in BRAMs_list:
    resource_report_file.write(ele + '\n')
  resource_report_file.write('DSPs-----------------------------------------\n')
  for ele in DSPs_list:
    resource_report_file.write(ele + '\n')

  resource_report_file.close()
  time_report_file.close()


 
  print("You can find the report under ./workspace/report")






