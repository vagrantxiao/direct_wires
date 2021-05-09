# prflow: This is all the source files for pr_flow
 Created by: Yuanlong Xiao (ylxiao@seas.upenn.edu)
         at: University of Pennsylvania
         in: 2019       


 The big picture:
 The prflow.py is the top function
 It generates several sub directories out of pr_flow directory
 
 How it works (in a nutshell):
 You need to install python2 and vivado 2018.2
 

 [1] D. Park, Y. Xiao and A. DeHon, "Case for Acceleration of FPGA Compilation using Partial Reconfiguration", FPL2018


## 1. benchmark setup
 1). Your hls files should be under ./input_files/hls_src/<benchmark name>
                                                                        /sdsoc
                                                                        /host

 2). Your C testbench should have the suffix of "underline+host.cpp" such as "face_detection_host.cpp"

 3). You can get some hints from other benchmarks such as "data_shift", "dg_reg", "face_detection", "rendering" and "spam_filter"

 4). You need to specify the "architecture.xml" for you benchmark.

 5). Mainly two parts of "architecture.xml" you shold modify.

 6). list all the functions names, input ports number, output number and page location in "function" items

 7). Specify all the connection information in "link" items.


## 2. Compile static region for the first time you run prflow.

 1). Run command line "python pr_flow.py <your benchmark name> -g -q" in icgrid.To start a quick demon, you can start with "make rendering"

 2). wait untill no qsub tasks in the grid.

 3). You can see you bits files under "../F005_bits" directory. run "./main.sh" under this directory, then you can download all the bits into ZCU102 board.
 4). Under directory "../F006_sdk/prj/floorplan.sdk", you can see an automatically generated hardware platform to generate sdk project.

## 3. Recomplile all the bench mark without recompiling the static region.
 1). Run command "python pr_flow.py <your benchmark name> -q" in icgrid

 2). wait untill no qsub tasks in the grid.

 3). You can see you regenrated bits files under "../F005_bits" directory. Run "./main.sh" under this directory, then you can download all the bits into ZCU102 board.

## 4. Recompile only one function
 1). If you only want make small changes to one function, you can run command "python pr_flow.py -f <your function name>

 2). Wait untill no qsub tasks in the grid, you can downlown the new bits into ZCU102 board



| #page | #LUTs | #FFs | #RAM18s | #DSPs |
|:---------:|:-----:|:----:|:-----:|:------:|
|2|5760|11520|24|72|
|3|5760|11520|24|72|
|4|5760|11520|24|72|
|5|5760|11520|24|72|
|6|5280|10560|48|24|
|7|5280|10560|48|24|
|8|5280|10560|48|24|
|9|5280|10560|48|24|
|10|4800|9600|48||
|11|4800|9600|48||
|12|4320|8640|48||
|13|4320|8640|48||
|14|3840|7680|24|24|
|15|4320|8640|48||
|16|4320|8640|48||
|17|4320|8640|48||
|18|5760|11520|48|48|
|19|5760|11520|48|48|
|20|5760|11520|24|72|
|21|5760|11520|24|72|
|22|5760|11520|48|48|
|23|5760|11520|48|48|
|24|5760|11520|48|48|
|25|5760|11520|48|48|
|26|5760|11520|48|48|
|27|5760|11520|48|48|
|28|5760|11520|48|48|
|29|5760|11520|48|48|
|30|5760|11520|48|48|
|31|5760|11520|48|48|

## 5. Benchmark Mapping

| #page | #LUTs | #FFs | #RAM18s | #DSPs |
|:---------:|:-----:|:----:|:-----:|:------:|
|2||5760|11520|24|72|
|3||5760|11520|24|72|
|4||5760|11520|24|72|
|5||5760|11520|24|72|
|6||5280|10560|48|24|
|7||5280|10560|48|24|
|8||5280|10560|48|24|
|9||5280|10560|48|24|
|10||4800|9600|48||
|11||4800|9600|48||
|12||4320|8640|48||
|13||4320|8640|48||
|14||3840|7680|24|48|
|15||4320|8640|48||
|16||4320|8640|48||
|17||4320|8640|48||
|18||5760|11520|48|48|
|19||5760|11520|48|48|
|20||5760|11520|24|72|
|21||5760|11520|24|72|
|22||5760|11520|48|48|
|23||5760|11520|48|48|
|24||5760|11520|48|48|
|25||5760|11520|48|48|
|26||5760|11520|48|48|
|27||5760|11520|48|48|
|28||5760|11520|48|48|
|29||5760|11520|48|48|
|30||5760|11520|48|48|
|31||5760|11520|48|48|





