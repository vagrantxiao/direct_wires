# -*- coding: utf-8 -*-   

import os  
import subprocess

class gen_hls:
  def __init__(self, prflow_params):
    self.prflow_params = prflow_params
    self.hls_dir = self.prflow_params['workspace']+'/F002_hls_'+self.prflow_params['benchmark_name']


  def file_name(self, file_dir):   
    for root, dirs, files in os.walk(file_dir):  
      print(root)
      print(dirs)  
      print(files)

  def get_file_name(self, file_dir):   
    for root, dirs, files in os.walk(file_dir):
      return files

  def gen_hls_files(self):
    if self.prflow_params['hls_regen']=='1':
      os.system('rm -rf ' + self.hls_dir)
      os.system('mkdir ' + self.hls_dir)



    
    os.chdir(self.hls_dir)
    main_sh = open('./main.sh', 'w')
    qsub_main_sh = open('./qsub_main.sh', 'w')
    main_sh.write('#!/bin/bash -e\n')
    qsub_main_sh.write('#!/bin/bash -e\n') 
    qsub_main_sh.write('emailAddr=\"'+self.prflow_params['email']+'\"\n\n')
    hls_fun_list = self.prflow_params['hls_fun_list']
    # hls_fun_list.append(self.prflow_params['mono_function'])
    iter_num = 1
    for fun_name in hls_fun_list:
      #generate the sh files
      run_sh = open('./run_' + fun_name + '.sh', 'w')
      run_sh.write('source ' + self.prflow_params['Xilinx_dir'] + '\n') 
      run_sh.write('vivado_hls -f ./' + fun_name + '_prj/' + fun_name + '/script.tcl\n') 
      run_sh.close()
      os.system('chmod +x ./run_' + fun_name + '.sh')
      qsub_run_sh = open('./qsub_run_' + fun_name + '.sh', 'w')
      qsub_run_sh.write('#!/bin/bash -e\n')
      qsub_run_sh.write('module load ' + self.prflow_params['qsub_Xilinx_dir'] + '\n') 
      qsub_run_sh.write('srun vivado_hls -f ./' + fun_name + '_prj/' + fun_name + '/script.tcl\n') 
      qsub_run_sh.close()
      os.system('chmod +x ./qsub_run_' + fun_name + '.sh')
      if iter_num % int(self.prflow_params['maxThreads']) == 0:
        main_sh.write('./run_' + fun_name +'.sh\n')  
      elif iter_num == len(hls_fun_list):
        main_sh.write('./run_' + fun_name +'.sh\n')  
      else:
        main_sh.write('./run_' + fun_name +'.sh&\n')  
      qsub_main_sh.write('sbatch --ntasks=1 --cpus-per-task=8 --mem-per-cpu=7500mb --job-name=hls_'+fun_name \
                         +  ' --dependency=$(squeue --noheader --format %i --user=$USER --name dummy | sed -n -e \'H;${x;s/\\n/,/g;s/^,//;p;}\')' \
                         + '  ./qsub_run_' + fun_name +'.sh\n')
      iter_num += 1

      #generate project files for each function
      #the reason is that one vivado project can only have on active hardware function
      #to be implemented
      os.system('mkdir ' + fun_name +'_prj')
      vivado_hls_app=open('./' + fun_name + '_prj/vivado_hls.app', 'w')
      vivado_hls_app.write('<project xmlns="com.autoesl.autopilot.project" name="' + fun_name + '_prj" top="'+ self.prflow_params['hls_fun_list'][0] + '">\n')
      vivado_hls_app.write('    <includePaths/>\n')
      vivado_hls_app.write('    <libraryPaths/>\n')
      vivado_hls_app.write('    <Simulation>\n')
      vivado_hls_app.write('        <SimFlow askAgain="false" name="csim" csimMode="0" lastCsimMode="0"/>\n')
      vivado_hls_app.write('    </Simulation>\n')
      vivado_hls_app.write('    <files xmlns="">\n')
      
      #capture all the files under host dirctory
      hls_src_list=self.get_file_name('../../input_files/hls_src/' + self.prflow_params['benchmark_name'] + '/host')
      for src_name in hls_src_list:
        if src_name.endswith('host.cpp'):
          vivado_hls_app.write('        <file name="../../input_files/hls_src/' + \
          self.prflow_params['benchmark_name'] + '/host/' + src_name + '" sc="0" tb="1" cflags=" -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas"/>\n')
        else:
          vivado_hls_app.write('        <file name="../../input_files/hls_src/' \
          + self.prflow_params['benchmark_name'] + '/host/' + src_name + '" sc="0" tb="false" cflags=""/>\n')
      
      #capture all the files under sdsoc dirctory
      hls_src_list=self.get_file_name('../../input_files/hls_src/' + self.prflow_params['benchmark_name'] + '/sdsoc')
      for src_name in hls_src_list:
        vivado_hls_app.write('        <file name="../../input_files/hls_src/' + self.prflow_params['benchmark_name'] + '/sdsoc/' + src_name + '" sc="0" tb="false" cflags=""/>\n')

      vivado_hls_app.write('    </files>\n    <solutions xmlns="">\n')  
      vivado_hls_app.write('        <solution name="' + fun_name + '" status="active"/>\n')
      vivado_hls_app.write('    </solutions>\n')
      vivado_hls_app.write('</project>\n')
      vivado_hls_app.close()
    
      #generate the hls script for each hardware function
      fun_dir = fun_name + '_prj/' + fun_name
      os.system('rm -rf ./' + fun_dir)
      os.system('mkdir ./' + fun_dir)
      script_file=open('./' + fun_dir + '/script.tcl', 'w')
      script_file.write('set logFileId [open ./runLog' + fun_name + '.log "a"]\n')
      script_file.write('set_param general.maxThreads ' + self.prflow_params['maxThreads'] + ' \n')
      script_file.write('set start_time [clock seconds]\n')
      script_file.write('open_project ' + fun_name + '_prj\n')
      script_file.write('set_top ' + fun_name + '\n')
      hls_src_list=self.get_file_name('../../input_files/hls_src/' + self.prflow_params['benchmark_name'] + '/host')
      #capture all the files under host dirctory
      for src_name in hls_src_list:
        if src_name.endswith('host.cpp'):
          script_file.write('add_files -tb ../../input_files/hls_src/' + self.prflow_params['benchmark_name'] +\
          '/host/' + src_name + ' -cflags "-Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas"\n')
        else:
          script_file.write('add_files ../../input_files/hls_src/' + self.prflow_params['benchmark_name'] + '/host/' + src_name + '\n')
      
      #capture all the files under sdsoc dirctory
      hls_src_list=self.get_file_name('../../input_files/hls_src/' + self.prflow_params['benchmark_name'] + '/sdsoc')
      for src_name in hls_src_list:
        script_file.write('add_files ../../input_files/hls_src/' + self.prflow_params['benchmark_name'] + '/sdsoc/' + src_name + '\n')
      script_file.write('open_solution "' +fun_name +'"\n')
      script_file.write('set_part {'+self.prflow_params['part']+'}\n')
      script_file.write('create_clock -period '+self.prflow_params['clk_user']+' -name default\n')
      script_file.write('#source "./Rendering_hls/colorFB/directives.tcl"\n')
      script_file.write('#csim_design\n')
      script_file.write('csynth_design\n')
      script_file.write('#cosim_design -trace_level all -tool xsim\n')
      #if(fun_name == self.prflow_params['mono_function']):
      #  script_file.write('export_design -rtl verilog -format ip_catalog\n')
      #else:
      #  script_file.write('#export_design -rtl verilog -format ip_catalog\n')
      
      script_file.write('export_design -rtl verilog -format ip_catalog\n')

      script_file.write('set end_time [clock seconds]\n')
      script_file.write('set total_seconds [expr $end_time - $start_time]\n')
      script_file.write('puts $logFileId "hls: $total_seconds seconds"\n')
      script_file.write('')
      script_file.write('exit\n')
      script_file.close()
    main_sh.close()

    os.system('chmod +x ./main.sh')
    qsub_main_sh.close()
    os.system('chmod +x ./qsub_main.sh')
    if self.prflow_params['run_qsub']:
        os.system('./qsub_main.sh')
    os.chdir('../../')



