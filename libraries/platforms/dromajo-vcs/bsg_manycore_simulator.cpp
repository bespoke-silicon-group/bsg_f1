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

// This file implements the SimulatorWrapper object, which hides
// differences in simulators.

#include <bsg_manycore_simulator.hpp>
#include <bsg_nonsynth_dpi_manycore.hpp>
#include <bsg_nonsynth_dpi_errno.hpp>
#include <bsg_nonsynth_dpi_cycle_counter.hpp>
#include <bsg_nonsynth_dpi_clock_gen.hpp>

#include <bsg_manycore.h>
#include <bsg_manycore_config.h>

#include "dromajo_cosim.h"
#include "dromajo_manycore.h"

#include <stdlib.h>
#include <stdio.h>

#include <svdpi.h>
// We use __m128i so that we can pass a 128-bit type between Verilog
// and C.
#include <xmmintrin.h>

extern "C" {
  void bsg_dpi_next();
}

// Dromajo specifics
dromajo_cosim_state_t *dromajo_state;
dromajo_cosim_state_t* dromajo_init();
bool dromajo_step();
void dromajo_transmit_packet();
void dromajo_receive_packet();
void dromajo_set_credits();

// DPI
bsg_nonsynth_dpi::dpi_manycore<HB_MC_CONFIG_MAX> *dpi;

SimulationWrapper::SimulationWrapper(){
	root = new std::string("replicant_tb_top");
	std::string mc_dpi = *root + ".mc_dpi";
	top = svGetScopeFromName(mc_dpi.c_str());
}

// Does nothing. Turning on/off assertions is only supported in
// Verilator.
void SimulationWrapper::assertOn(bool val){

}

std::string SimulationWrapper::getRoot(){
  return *root;
}

void SimulationWrapper::eval(){
	// Execute one instruction on Dromajo
	if (!dromajo_step()) {
		// Fixme: Dromajo could also terminate due to premature errors.
		// Use the test outputs to detect PASS/FAIL
		printf("Dromajo Execution Complete!\nExiting...\n");
		dromajo_cosim_fini(dromajo_state);
		exit(0);
	}

	// Poll for packets to be transmitted
	dromajo_transmit_packet();
	// Poll for packets to be received
	dromajo_receive_packet();
	// Update the credits in dromajo
	dromajo_set_credits();

	// Advance time 1 unit
	svScope prev;
	prev = svSetScope(top);
	bsg_dpi_next();
	svSetScope(prev);
}

SimulationWrapper::~SimulationWrapper(){
	this->top = nullptr;
}

static int dpi_fifo_drain(bsg_nonsynth_dpi::dpi_manycore<HB_MC_CONFIG_MAX> *dpi, hb_mc_fifo_rx_t type) {
	__m128i *pkt;

	int err, drains = 0;
	int cap = dpi->config[HB_MC_CONFIG_IO_REMOTE_LOAD_CAP];

	do {
		bsg_dpi_next();
		switch(type) {
			case HB_MC_FIFO_RX_REQ:
				err = dpi->rx_req(*pkt);
			break;
			case HB_MC_FIFO_RX_RSP:
				err = dpi->rx_rsp(*pkt);
			break;
			default:
				printf("Unknown FIFO type\n");
				return HB_MC_FAIL;
		}

		if (err == BSG_NONSYNTH_DPI_SUCCESS)
			drains++;
	} while ((err == BSG_NONSYNTH_DPI_NOT_WINDOW | err == BSG_NONSYNTH_DPI_SUCCESS)
					&& drains <= cap);

	if (drains != cap)
		printf("Failed to drain FIFO\n");
		return HB_MC_FAIL;

	return HB_MC_SUCCESS;
}

static void dpi_clean(bsg_nonsynth_dpi::dpi_manycore<HB_MC_CONFIG_MAX> *dpi) {
	delete dpi;
}

static int dpi_init(bsg_nonsynth_dpi::dpi_manycore<HB_MC_CONFIG_MAX> *dpi, std::string hierarchy) {
	bsg_dpi_next();

	dpi = new bsg_nonsynth_dpi::dpi_manycore<HB_MC_CONFIG_MAX>(hierarchy + ".mc_dpi");

	int err;
	err = dpi_fifo_drain(dpi, HB_MC_FIFO_RX_REQ);
	if (err != HB_MC_SUCCESS) {
		dpi_clean(dpi);
		return err;
	}

	err = dpi_fifo_drain(dpi, HB_MC_FIFO_RX_RSP);
	if (err != HB_MC_SUCCESS) {
		dpi_clean(dpi);
		return err;
	}

	return HB_MC_SUCCESS;
}

static int dpi_wait_for_reset(bsg_nonsynth_dpi::dpi_manycore<HB_MC_CONFIG_MAX> *dpi) {
	bool done;
	dpi->reset_is_done(done);

	while (!done) {
		bsg_dpi_next();
		dpi->reset_is_done(done);
	}

	return HB_MC_SUCCESS;
}

dromajo_cosim_state_t* dromajo_init() {
	dromajo_cosim_state_t *dromajo_state;
	
	char dromajo[50];
	sprintf(dromajo, "dromajo");

	char host[50];
	sprintf(host, "--host");
	
	char manycore[50];
	sprintf(manycore, "--manycore");

	char progname[50];
	sprintf(progname, "test.elf");

	char* argv[] = {dromajo, host, manycore, progname};
	dromajo_state = dromajo_cosim_init(4, argv);

	return dromajo_state;
}

bool dromajo_step() {
	// Execute dromajo with verbose mode on
	int err = dromajo_cosim_step(dromajo_state, 0, 0, 0, 0, 0, false, true);
	if (err != 0)
		return false;
	else
		return true;
}

void dromajo_transmit_packet() {
	hb_mc_packet_t *packet;
	int err;
	__m128i *pkt;

	// Check if FIFO is full and hence ready to transmit
	mc_fifo_type_t type = FIFO_HOST_TO_MC_REQ;
	bool is_full = mc_is_fifo_full(type);

	if (is_full) {
		// Read the FIFO head pointer for all 32-bit FIFOs
		packet->words[0] = host_to_mc_req_fifo->fifo[0].front();
		packet->words[1] = host_to_mc_req_fifo->fifo[1].front();
		packet->words[2] = host_to_mc_req_fifo->fifo[2].front();
		packet->words[3] = host_to_mc_req_fifo->fifo[3].front();

		pkt = reinterpret_cast<__m128i *>(packet);

		// Try to transmit until you actually can
		err = dpi->tx_req(*pkt);

		// Drain the FIFO once transmitted
		if (err == BSG_NONSYNTH_DPI_SUCCESS) {
			for (int i = 0;i < 4; i++)
				host_to_mc_req_fifo->fifo[i].pop();
		}
	}
}

void dromajo_receive_packet() {
	hb_mc_packet_t *req_packet, *resp_packet;
	int err;
	__m128i *pkt;

	// Read from the manycore request FIFO
	err = dpi->rx_req(*pkt);
	req_packet = reinterpret_cast<hb_mc_packet_t *>(pkt);

	// Push to host if valid and mark the FIFOs as full
	if (err == BSG_NONSYNTH_DPI_SUCCESS) {
		for (int i = 0; i < 4; i++) {
			mc_to_host_req_fifo->fifo[i].push(req_packet->words[i]);
			mc_to_host_req_fifo->full[i] = true;
		}
	}

	// Read from the manycore response FIFO
	err = dpi->rx_rsp(*pkt);
	resp_packet = reinterpret_cast<hb_mc_packet_t *>(pkt);

	// Push to host if valid and mark the FIFOs as full
	if (err == BSG_NONSYNTH_DPI_SUCCESS) {
		for (int i = 0; i < 4; i++) {
			mc_to_host_resp_fifo->fifo[i].push(resp_packet->words[i]);
			mc_to_host_resp_fifo->full[i] = true;
		}
	}
}

void dromajo_set_credits() {
	int credits;
	int res = dpi->get_credits(credits);
	if (res == BSG_NONSYNTH_DPI_SUCCESS) {
		if (credits < 0)
			printf("Warning! Credit value is negative!\n");

		host_to_mc_req_fifo->credits = credits;
	}
}

#ifdef VCS
int vcs_main(int argc, char **argv) {
#else
int main(int argc, char **argv) {
#endif

	// Initialize Dromajo
	dromajo_state = dromajo_init();

	// Create an object of the simulation wrapper
	SimulationWrapper *sim = new SimulationWrapper();
	std::string hierarchy = sim->getRoot();

	int err;

	// Initialize DPI
	err = dpi_init(dpi, hierarchy);
	if (err != HB_MC_SUCCESS) {
		delete sim;
		printf("Failed to initialize DPI!\nExiting...\n");
		return -1;
	}

	// Wait for reset to be done
	err = dpi_wait_for_reset(dpi);
	if (err != HB_MC_SUCCESS) {
		dpi_clean(dpi);
		delete sim;
		printf("Failed to wait for reset!\nExiting...\n");
		return -1;
	}
	
	while(1)
		sim->eval();

	return 0;
}