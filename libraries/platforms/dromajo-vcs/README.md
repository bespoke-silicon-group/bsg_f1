# Dromajo HammerBlade Platform on VCS

## Introduction

The Dromajo-HammerBlade platform is a precursor to the BlackParrot-HammerBlade platform a.k.a HammerParrot. It builds on the traditional x86 setup and replaces x86 with Dromajo as the main host (except for a few operations) that executes the CUDA-lite program. Broadly speaking there are three main components to the entire system

- x86: It is the base platform on which the entire simulation infrastructure is built. In a traditional setup, the x86 core runs the CUDA-lite binary while simulating the manycore hardware. In this case, the x86 core instead runs the Dromajo emulator in addition to simulating the manycore hardware whilst performing some host-like operations and interacting with the hardware through DPI.
- Dromajo: A C++ based 64-bit RISC-V processor emulator developed by Esperanto Technologies with modifications to communicate with the HammerBlade manycore. All CUDA-lite host code is executed by a RISC-V processor emulated by Dromajo. Since, HammerParrot is the eventual goal, Dromajo has been modified to work like BlackParrot.
- HammerBlade hardware: Like all other platforms, the manycore hardware consists of the manycore with a specific machine configuration and is equipped with DPI FIFOs on the west side forming the manycore bridge.

## Dependencies

### BlackParrot SDK

The BlackParrot SDK contains multiple submodules
- Dromajo
- Perch, the BlackParrot firmware
- RV64 compilation toolchain

This directory must be placed at the same level as the replicant repository. It is recommended that users clone the Bladerunner meta repository which includes (or will include) the BlackParrot SDK as a submodule [(PR #66)](https://github.com/bespoke-silicon-group/bsg_bladerunner/pull/66) since this will set all the required environment variables correctly. Otherwise users must define the BLACKPARROT_SDK_DIR variable for the given execution environment.

## Detailed Technical Manual

Coming soon!

## Using the Simulation Infrastructure

The simulation infrastructure can be used just like any other platform. An example is given below.

```
cd examples/library/test_coordinate
make main.exec.log
```

This will build the libraries and the platform binaries, compile and elaborate the hardware and run the test program. The example output for the above program should look as given below

```
BSG INFO: Regression Test: test_coordinate
INFO:    Starting test foreach_coordinate
INFO:    iteration  0: (0,0)
INFO:    Starting test foreach_x_y
INFO:    iteration  0: (0,0)
INFO:    Starting test foreach_coordinate
INFO:    iteration  0: (0,0)
INFO:    iteration  1: (1,0)
INFO:    Starting test foreach_x_y
INFO:    iteration  0: (0,0)
INFO:    iteration  1: (1,0)
INFO:    Starting test foreach_coordinate
INFO:    iteration  0: (0,0)
INFO:    iteration  1: (0,1)
INFO:    Starting test foreach_x_y
INFO:    iteration  0: (0,0)
INFO:    iteration  1: (0,1)
INFO:    Starting test foreach_coordinate
INFO:    iteration  0: (0,2)
INFO:    iteration  1: (0,3)
INFO:    iteration  2: (1,2)
INFO:    iteration  3: (1,3)
INFO:    Starting test foreach_x_y
INFO:    iteration  0: (0,2)
INFO:    iteration  1: (0,3)
INFO:    iteration  2: (1,2)
INFO:    iteration  3: (1,3)
BSG REGRESSION TEST PASSED
INFO:    Core 0 successfully terminated
BSG REGRESSION TEST PASSED
BSG COSIM PASS: Test passed!
```

### Notes

- This infrastructure has only been tested with tests in `examples/library`. SPMD and CUDA-lite testing is still remaining.
- All testing has been carried out for a single pod configuration.
- Only the `exec` make target has been tested. The `saif` and `debug` targets still remain to be tested. However, since this pertains to gathering metrics with respect to the hardware being simulated, there is a fair amount of confidence that it will work right out of the box.
- The simulation takes a very long time even for some of the library tests. This can be attributed to the fact that Dromajo is used in co-simulation mode (one instruction per time step) with code executing on the x86 core controlling the notion of time for the whole system. Dromajo also has a standalone mode in which case it must become the driver of time for the whole system. While the reason mentioned here should be verified after an in-depth performance analysis of the system, it is still worthwhile to explore Dromajo in the standalone mode. This is one of major barriers to running CUDA-lite test programs and completing its execution in a reasonable time frame.
