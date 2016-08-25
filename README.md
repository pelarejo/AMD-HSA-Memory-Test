# AMD-HSA-Memory-Test
This repository hosts AMD-HSA-Memory-Test designed to test AMD's implementation of HSA on the Carrizo APU architecture.

HSA is a system architecture designed to harvest the most out of processing units by introducing a common memory pool accessible to both the GPU and the CPU.
However, concurrent memory models often comes with relaxed behaviours which can be unintuitive and poorly documented. This program is meant to provide a first step for stress testing the APU in an effort of documenting an accurate behaviour.

## How it works
AMD-HSA-Memory-Test relies heavily on HSAIL, the intermediate language  between agents and the higher level compiler, in order to have a more granular control of HSA by accessing processors at a low level. This language gets compiled into BRIG files using the ROCm provided HSAILasm compiler and are binaries capable of being loaded by a host program.

The host program is done in C and uses ROCm provided libraries to initialise, load, finalise and run HSAIL programs any number of times. Documentation is provided on the [HSA Foundation] (http://www.hsafoundation.com/standards/) website under HSA Runtime Specification 1.1.

HSAIL programs are being search in the hsail/ folder under ./AMD-HSA-Memory-Test/test_suite/. They are unit tests which take an input and output arrays and operates on those array concurrently in an effort to trigger unexpected behaviours. References to HSAIL can be found on the [HSA Foundation] (http://www.hsafoundation.com/standards/) website under HSA Programmer Reference Manual Specification 1.1.

## Requirements

AMD-HSA-Memory-Test suite has been built around the ROCm platform provided by AMD and has been tested with the following configuration:

* Aspire E5-552G with
	* AMD FX-8800P APU (Carizzo)
	* dGPU R8 M365DX (disabled)
* Ubuntu 14.04 Trusty
* ROCm platform 1.1.1 available [here] (https://github.com/RadeonOpenCompute/ROCm)
* GCC

It would potentially work any laptop or desktop equipped with AMD's Carizzo and Kaveri APU.

### Installation

1. Install and boot on the ROCm Kernel

	```shell
	// On Ubuntu:
	sudo apt-get install rocm
	```

2. Verify if the active GPU is from a compatible family

	```shell
	lspci -vnnn | perl -lne 'print if /^\d+\:.+(\[\S+\:\S+\])/' | grep VGA
	```

3. Verify if the active driver is amdgpu

	```shell
	sudo lshw -c video
	```

4. Build the test suite
	
	```shell
	cd ./AMD-HSA-Memory-Test/test_suite/
	./compile_hsail.sh		// Assuming HSAILasm is in PATH
	make
	```
5. Run

	```shell
	./memory_test_hsa [-vah] [-n file_name -r regs [-i runs_nbr]]
	```
## Links
Github: https://github.com/Py0s/AMD-HSA-Memory-Test  
HSA Foundation: http://www.hsafoundation.com/standards/  
ROCm: https://github.com/RadeonOpenCompute/ROCm