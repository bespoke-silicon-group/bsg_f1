// Copyright (c) 2020, University of Washington All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this list
// of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
//
// Neither the name of the copyright holder nor the names of its contributors may
// be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// SimulatorWrapper hides Vreplicant_tb_top behind a void *. 

#include <bsg_manycore_simulator.hpp>
#include <bsg_nonsynth_dpi_clock_gen.hpp>
#include <verilated.h>
#include <Vreplicant_tb_top.h>

SimulationWrapper::SimulationWrapper(){
        Vreplicant_tb_top *top = new Vreplicant_tb_top();
        this->top = reinterpret_cast<void *>(top);
        
        // When Verilator simulation starts, we want to disable
        // assertion because it is a two-state simulator and the lack
        // of z/x may cause erroneous assertions.
        Verilated::assertOn(false);

        // Uncomment THIS and the statement in verilator_top.sv to enable tracing
        // Verilated::traceEverOn(true);

        top->eval();
        root = new std::string("TOP.replicant_tb_top");
}

// Change the assertion state. This does not need to be implemented in
// 4-state simulators like VCS
void SimulationWrapper::assertOn(bool val){
        Verilated::assertOn(val);
}

// Cause time to proceed. 
void SimulationWrapper::eval(){
        Vreplicant_tb_top *top = reinterpret_cast<Vreplicant_tb_top *>(this->top);
        bsg_nonsynth_dpi::bsg_timekeeper::next();
        top->eval();
}

std::string SimulationWrapper::getRoot(){
        return *root;
}

SimulationWrapper::~SimulationWrapper(){
        Vreplicant_tb_top *top = reinterpret_cast<Vreplicant_tb_top *>(this->top);
        delete top;
        this->top = nullptr;
}

