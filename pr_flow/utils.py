#!/usr/bin/env python
import sys
import os
import xml.etree.ElementTree
import argparse
import re
import math

# parse the list number according to the direction
# WEST:  0
# NORTH: 1
# SOUTH: 2
# EAST:  3
def return_direction_num(direction):
  direction_dict = {'W': 0, 'N': 1, 'S': 2, 'E': 3}
  return direction_dict[direction]


# translate the function name to page name if necessary
def return_page(str_in, mapping_dict):
  if str_in.startswith('X'):
    return str_in
  else:
    return mapping_dict[str_in]


# return out_edge, in_edge
def return_IO_edge(src_page, dst_page):

  X_src = int(src_page.split('X')[1].split('Y')[0])
  Y_src = int(src_page.split('X')[1].split('Y')[1])
  X_dst = int(dst_page.split('X')[1].split('Y')[0])
  Y_dst = int(dst_page.split('X')[1].split('Y')[1])
  if X_src == X_dst:
    if Y_src == Y_dst+1:
      return return_direction_num('S'), return_direction_num('N') 
    elif Y_src == Y_dst-1:
      return return_direction_num('N'), return_direction_num('S')
    else:
      return 'Error', 'Error'
  elif Y_src == Y_dst:
    if X_src == X_dst+1:
      return return_direction_num('W'), return_direction_num('E') 
    elif X_src == X_dst-1:
      return return_direction_num('E'), return_direction_num('W')
    else:
      return 'Error', 'Error'
  else:
    return 'Error', 'Error'


# calculate the minumum datawidht for all the meash pages
def gen_min_width(links, mapping_dict):
  width_in_dict = {}
  width_out_dict = {}
  width_max_dict = {}
  for i in range(4):
    for j in range(7):
      width_in_dict['X'+str(i)+'Y'+str(j)] = [0,0,0,0]
      width_out_dict['X'+str(i)+'Y'+str(j)] = [0,0,0,0]

  for child in links:
    link_chain = child.get('source').split('-')
    for i in range(len(link_chain)-1):
      src_page  = return_page(link_chain[i].split('.')[0], mapping_dict)
      dest_page = return_page(link_chain[i+1].split('.')[0], mapping_dict)
      out_dir, in_dir = return_IO_edge(src_page, dest_page)
      width = int(child.get('width'))
      width_out_dict[src_page][out_dir]+=width 
      width_out_dict[src_page][out_dir]+=2 
      width_in_dict[dest_page][in_dir]+=width 
      width_in_dict[dest_page][in_dir]+=2 

  for key in width_in_dict: 
    value_in = width_in_dict[key]
    value_out = width_out_dict[key]
    for i in range(len(value_in)):
      if value_in[i] < value_out[i]:
        value_in[i] = value_out[i]
      if value_in[i] == 0:
        value_in[i] = 32
    width_max_dict[key] = value_in
  
  return width_max_dict


def load_prflow_params(filename):
  prflow_params = {
    'nl': -1,
    'p': -1,
    'pks': -1,
    'bft_gen': 0,
    'static_run': 0
  }

  root = xml.etree.ElementTree.parse('common/configure/configure.xml').getroot()
  specs = root.findall('spec')
  clock =root.findall('clock')
  datawidth_in=root.findall('datawidth_in')
  datawidth_out=root.findall('datawidth_out')
  place_holder=root.findall('place_holder')
  network=root.findall('network')

  root = xml.etree.ElementTree.parse(filename).getroot()
  functions = root.findall('function')
  links = root.findall('link')	

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


#  for key in datawidth_dict.keys():
#    print '<network pe = \''+key+'\' W = \''+str(datawidth_dict[key][0])+'\' N = \''+str(datawidth_dict[key][1])+'\' S = \''+str(datawidth_dict[key][2])+'\' E = \''+str(datawidth_dict[key][3])+'/>' 




 
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


  return prflow_params



