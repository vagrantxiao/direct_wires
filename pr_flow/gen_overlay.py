# -*- coding: utf-8 -*-   

import os  
import subprocess

class gen_overlay:
  def __init__(self, prflow_params):
    self.prflow_params = prflow_params
    self.overlay_dir = 'F004_mk_overlay'

  def overlay_gen(self):
    os.system('rm -rf ../' + self.overlay_dir)
    os.system('mkdir ../' + self.overlay_dir)

    run_file=open('../' + self.overlay_dir + '/run.sh', 'w')
    run_file.write('#!/bin/bash -e\n')
    run_file.write('source ' + self.prflow_params['Xilinx_dir'] + '\n')
    run_file.write('vivado -mode batch -source impl_static_' + str(self.prflow_params['nl']) + '_48.tcl\n')
    run_file.close()
    os.system('chmod +x ' + '../' + self.overlay_dir + '/run.sh')

    qsub_run_file=open('../' + self.overlay_dir + '/qsub_run.sh', 'w')
    qsub_run_file.write('#!/bin/bash -e\n')
    qsub_run_file.write('source ' + self.prflow_params['qsub_Xilinx_dir'] + '\n')
    qsub_run_file.write('vivado -mode batch -source impl_static_' + str(self.prflow_params['nl']) + '_48.tcl\n')
    qsub_run_file.close()
    os.system('chmod +x ' + '../' + self.overlay_dir + '/qsub_run.sh')



    tcl_file=open('../' + self.overlay_dir + '/impl_static_' + str(self.prflow_params['nl']) + '_48.tcl', 'w')
    tcl_file.write('set logFileId [open ./runLog_impl_big_static_' + str(self.prflow_params['nl']) + '_48.log "a"]\n')
    tcl_file.write('\n')
    tcl_file.write('#####################\n')
    tcl_file.write('## read_checkpoint ##\n')
    tcl_file.write('#####################\n')
    tcl_file.write('set start_time [clock seconds]\n')
    tcl_file.write('open_checkpoint ../F001_static_' + str(self.prflow_params['nl']) + '_leaves/floorplan_static.dcp\n')
    for i in range(2, int(self.prflow_params['nl'])):
      tcl_file.write('read_checkpoint -cell floorplan_static_i/leaf_empty_' + str(i) + '/inst ../F003_syn_leaf/user_kernel/user_kernel_leaf_netlist_48.dcp\n')
    tcl_file.write('set end_time [clock seconds]\n')
    tcl_file.write('set total_seconds [expr $end_time - $start_time]\n')
    tcl_file.write('puts $logFileId "read_checkpoint: $total_seconds seconds"\n')
    tcl_file.write('\n')
    tcl_file.write('####################\n')
    tcl_file.write('## implementation ##\n')
    tcl_file.write('####################\n')
    tcl_file.write('set start_time [clock seconds]\n')
    tcl_file.write('opt_design\n')
    tcl_file.write('set end_time [clock seconds]\n')
    tcl_file.write('set total_seconds [expr $end_time - $start_time]\n')
    tcl_file.write('puts $logFileId "opt: $total_seconds seconds"\n')
    tcl_file.write('set start_time [clock seconds]\n')
    tcl_file.write('place_design\n')
    tcl_file.write('set end_time [clock seconds]\n')
    tcl_file.write('set total_seconds [expr $end_time - $start_time]\n')
    tcl_file.write('puts $logFileId "place: $total_seconds seconds"\n')
    tcl_file.write('# write_hwdef -force pr_test_wrapper.hwdef\n')
    tcl_file.write('write_checkpoint -force init_placed_' + str(self.prflow_params['nl']) + '_48.dcp\n')
    tcl_file.write('set start_time [clock seconds]\n')
    tcl_file.write('route_design\n')
    tcl_file.write('set end_time [clock seconds]\n')
    tcl_file.write('set total_seconds [expr $end_time - $start_time]\n')
    tcl_file.write('puts $logFileId "route: $total_seconds seconds"\n')
    tcl_file.write('write_checkpoint -force init_routed_' + str(self.prflow_params['nl']) + '_48.dcp\n')
    tcl_file.write('set_param bitstream.enablePR 2341\n')
    tcl_file.write('write_bitstream -force -no_partial_bitfile  ../F006_bits/main.bit\n')
    tcl_file.write('#############################################\n')
    tcl_file.write('## create static design with no bft pblock ##\n')
    tcl_file.write('#############################################\n')
    tcl_file.write('\n')
    tcl_file.write('set start_time [clock seconds]\n')
    tcl_file.write('update_design -cell floorplan_static_i/bft_dma -black_box\n')
    for i in range(2, int(self.prflow_params['nl'])):
      tcl_file.write('update_design -cell floorplan_static_i/leaf_empty_' + str(i) + '/inst -black_box\n')
    
    tcl_file.write('update_design -cell floorplan_static_i/bft_dma -buffer_ports\n')
    for i in range(2, int(self.prflow_params['nl'])):
      tcl_file.write('update_design -cell floorplan_static_i/leaf_empty_' + str(i) + '/inst -buffer_ports\n')
    
    tcl_file.write('lock_design -level routing\n')
    tcl_file.write('write_checkpoint big_static_routed_' + str(self.prflow_params['nl']) + '_48.dcp\n')
    tcl_file.write('close_design\n')
    tcl_file.write('set end_time [clock seconds]\n')
    tcl_file.write('set total_seconds [expr $end_time - $start_time]\n')
    tcl_file.write('puts $logFileId "update, black_box: $total_seconds seconds"\n')
    tcl_file.write('# set start_time [clock seconds]\n')
    tcl_file.write('# set end_time [clock seconds]\n')
    tcl_file.write('# set total_seconds [expr $end_time - $start_time]\n')
    tcl_file.write('# puts $logFileId "write bitstream: $total_seconds seconds"\n')
    tcl_file.close()

    main_sh = open('../' + self.overlay_dir + '/main.sh', 'w')
    main_sh.write('#!/bin/bash -e\n')
    main_sh.write('./run.sh\n')
    main_sh.close()
    os.system('chmod +x ' + '../' + self.overlay_dir + '/main.sh')


    qsub_main_sh = open('../' + self.overlay_dir + '/qsub_main.sh', 'w')
    qsub_main_sh.write('#!/bin/bash -e\n') 
    qsub_main_sh.write('emailAddr=\"'+ self.prflow_params['email'] +'\"\n\n')
    qsub_main_sh.write('qsub -N mk_overlay -q ' + self.prflow_params['qsub_grid'] + ' -hold_jid syn_to_dcp,syn_user_kernel  -m abe -M $emailAddr  -l mem=8G  -cwd ./qsub_run.sh\n')
    qsub_main_sh.close()
    os.system('chmod +x ' + '../' + self.overlay_dir + '/qsub_main.sh')
    os.chdir('../' + self.overlay_dir)
    if self.prflow_params['run_qsub']:
        os.system('./qsub_main.sh')
    os.chdir('../pr_flow')


