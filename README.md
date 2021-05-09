# Direct Wires Parallel Mapping
Direct Wires (DW PRFlow) is a framework that allows FPGA developers to map their 
applications by using existed PRflow framework, but without NoC bandwidth limitations.

### How DW prflow works?
We quantize the MPSoC Fabrics into coarse-grained blocks as the figure below.
The application is coposed of operators, which are connected by streaming
interfaces.

![](images/overlay_new.jpg) 

When we map the application, the operators will be mapped to the separated
gridded fabrics. We will take the dataflow graph below as an example.

![](images/dfg_rendering.jpg) 
 

## 1. Getting Started
```
$ git clone https://github.com/RC4ML/Shuhai.git
$ git submodule update --init --recursive
```

## 2. Build FPGA Project
```
$ cd hw/
```
According to hw/README.md, build vivado project and program the FPGA with the generated bitstream. 

## 3. Build Software Project
```
$ cd sw/
```
According to sw/README.md, build the software project and run the application


## Frequently Asked Questions
1, Q. the machine failed to detect PCIe on the FPGA when loading the kernel module.

   A. Connect the JTAG to another machine that will not crash when downloading the FPGA image. It means that you cannot use the same machine to load the bitstream. 


## Cite this work
If you use it in your paper, please cite our work ([full version](https://ieeexplore-ieee-org.proxy.library.upenn.edu/document/9415587)).
```
@inproceedings{xiao2020fast,
  title={Fast Linking of Separately-Compiled FPGA Blocks without a NoC},
  author={Xiao, Yuanlong and Ahmed, Syed Tousif and DeHon, André},
  booktitle={2020 International Conference on Field-Programmable Technology (ICFPT)},
  volume={},
  number={},
  pages={196-205},
  doi={10.1109/ICFPT51103.2020.00035}
}

```
### Related publications
* Yuanlong Xiao, Syed Tousif Ahmedand, and Adnr\'e DeHon}. [Fast Linking of Separately-Compiled FPGA Blockswithout a NoC](doc/xiao2020fast.pdf). ICFPT, 2020.


