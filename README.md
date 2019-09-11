# BSG Manycore F1 Design (Bladerunner)

This repository contains the Amazon AWS F1 software and interface
logic for the Bespoke Silicon Group Manycore on AWS F1. 

## Contents

This repository contains the following folders: 

- `build`: Vivado scripts for building FPGA Design Checkpoint Files to upload to AWS-F1
- `hardware`: HDL sources, ROM scripts, and package files
- `libraries`: C/C++ driver and CUDA-lite Runtime library sources
- `regression`: Regression tests for cosimuation and AWS F1 Execution
- `testbenches`: Testbench files for RTL and C/C++ Co-Simulation
- `scripts`: Scripts used to upload Amazon FPGA images (AFIs) and configure Amazon Machine Images (AMIs).

This repository contains the following files:

- `Makefile`: Contains targets for Co-Simulation, Bitstream Generation, F1 Regression
- `Makefile.machine.include`: Defines the Manycore configuration for cosimulation and bitstream compilation
- `README.md`: This file
- `cadenv.mk`: A makefile fragment for deducing the CAD tool environment
- `environment.mk`: A makefile fragment for deducing the build environment. 
- `hdk.mk`: A makefile fragment for deducing the AWS-FPGA HDK build environment.

## Dependencies

To simulate/cosimulate/build these projects you must have the following tools.

   1. Vivado 2018.2
   2. A clone of aws-fpga (v1.4.5) 
   3. Synopsys VCS

This repository depends on the following repositories: 

   1. [BSG Manycore](https://github.com/bespoke-silicon-group/bsg_manycore)
   2. [BaseJump STL](https://github.com/bespoke-silicon-group/basejump_stl)
   3. [AWS FPGA](https://github.com/aws/aws-fpga)

# Quick-Start

The simplest way to use this project is to clone it's meta-project: [BSG Bladerunner](https://github.com/bespoke-silicon-group/bsg_bladerunner/). 

BSG Bladerunner tracks BSG F1 (this repository, BSG Manycore, and
BaseJump STL repositories as submodules and maintains a monotonic
versionining scheme. BSG Bladerunner also contains cosimulation
instructions. 
