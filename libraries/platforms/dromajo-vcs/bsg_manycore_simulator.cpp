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

#include <dromajo_cosim.h>
#include <dromajo_manycore.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <bsg_manycore_regression.h>

#include <svdpi.h>
// We use __m128i so that we can pass a 128-bit type between Verilog
// and C.
#include <xmmintrin.h>

// DPI function to advance time
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

// Global variables to store arguments
int _argc;
char **_argv;
static int char_index = 0;
int idx = 0;

// DPI object
bsg_nonsynth_dpi::dpi_manycore<HB_MC_CONFIG_MAX> *dpi;

// Define invisible library functions
declare_hb_mc_get_bits

////////////////////////////// SimulationWrapper functions //////////////////////////////

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

////////////////////////////// DPI helper functions //////////////////////////////

/*
 * dpi_fifo_drain
 * Drains all DPI fifos. This function is used during initialization and does not push packets
 * to the FIFOs in Dromajo
 * @param[in] dpi - Pointer to the manycore DPI object
 * @param[in] type - Manycore FIFO type
 * @returns success if all the FIFOs were drained correctly
 */
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
        printf("[BSG_ERROR] Unknown FIFO type\n");
        return HB_MC_FAIL;
    }

    if (err == BSG_NONSYNTH_DPI_SUCCESS)
      drains++;
  } while ((err == BSG_NONSYNTH_DPI_NOT_WINDOW | err == BSG_NONSYNTH_DPI_SUCCESS)
          && drains <= cap);

  if (drains != cap)
    printf("[BSG_ERROR] Failed to drain FIFO\n");
    return HB_MC_FAIL;

  return HB_MC_SUCCESS;
}

/*
 * dpi_clean
 * Destroys the pointer to the manycore DPI object
 * @params[in] dpi - Pointer to the manycore DPI object
 */
static void dpi_clean(bsg_nonsynth_dpi::dpi_manycore<HB_MC_CONFIG_MAX> *dpi) {
  delete dpi;
}

/*
 * dpi_init
 * Initializes the DPI between the manycore and Dromajo
 * @params[in] dpi - Pointer to the manycore DPI object
 * @params[in] hierarchy - A C++ string that holds the manycore hierarchy
 * @returns success if initialized correctly
 */
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

/*
 * dpi_wait_for_reset
 * Ensures that the manycore hardware is reset
 * @params[in] dpi - Pointer to the manycore DPI object
 */
static int dpi_wait_for_reset(bsg_nonsynth_dpi::dpi_manycore<HB_MC_CONFIG_MAX> *dpi) {
  bool done;
  dpi->reset_is_done(done);

  while (!done) {
    bsg_dpi_next();
    dpi->reset_is_done(done);
  }

  return HB_MC_SUCCESS;
}

////////////////////////////// Dromajo helper functions //////////////////////////////

/*
 * dromajo_init
 * Initializes dromajo with the correct command-line arguments
 * @returns a pointer to a dromajo_cosim_state_t object
 */
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

/*
 * dromajo_step
 * Executes 1 instruction in Dromajo
 * @returns true if execution is incomplete and/or without errors
 */
bool dromajo_step() {
  // Execute dromajo with verbose mode on
  int err = dromajo_cosim_step(dromajo_state, 0, 0, 0, 0, 0, false, true);
  if (err != 0)
    return false;
  else
    return true;
}

/*
 * dromajo_transmit_packet
 * Fetches data from the Dromajo->Manycore FIFO and pushes it into the DPI FIFO
 * to send packets to the manycore
 */
void dromajo_transmit_packet() {
  hb_mc_packet_t packet, host_resp_packet;
  int err;
  __m128i *pkt;

  // Check if FIFO is full and hence ready to transmit
  mc_fifo_type_t type = FIFO_HOST_TO_MC_REQ;
  bool is_full = mc_is_fifo_full(type);

  if (is_full) {
    // Read the FIFO head pointer for all 32-bit FIFOs
    packet.words[0] = host_to_mc_req_fifo->fifo[0].front();
    packet.words[1] = host_to_mc_req_fifo->fifo[1].front();
    packet.words[2] = host_to_mc_req_fifo->fifo[2].front();
    packet.words[3] = host_to_mc_req_fifo->fifo[3].front();

    // Intercept packets that are for the host and generate appropriate responses
    if ((packet.request.x_dst == 0) && (packet.request.y_dst == 0)) {
      host_resp_packet.response.x_dst = packet.request.x_src;
      host_resp_packet.response.y_dst = packet.request.y_src;
      host_resp_packet.response.load_id = 0;
      if (packet.request.op_v2 == HB_MC_PACKET_OP_REMOTE_LOAD) {
        host_resp_packet.response.op = packet.request.op_v2;
        idx = packet.request.addr;
        uint32_t data = 0;
        // If EPA maps to reading arguments
        if (packet.request.addr >= 0 && packet.request.addr <= 0xFF) {
          int num_characters = 0;
          // If all arguments have been read, send this finish code
          if (idx == _argc) {
            data = 0xFFFFFFFF;
          }
          else {
            // Copy 4 bytes of the arguments
            for(int i = 0; i < 4; i++) {
              if (_argv[idx][char_index] != '\0') {
                data = (data << 8) | _argv[idx][char_index];
                num_characters++;
                char_index++;
              }
              else {
                data = (data << (4 - num_characters) * 8);
                char_index = 0;
              }
            }
          }
        }
        // Dromajo/BlackParrot wants to read the config
        else if (packet.request.addr >= 0x100 && packet.request.addr <= 0x1FF) {
          idx = packet.request.addr;
          data = dpi->config[idx];
        }
        else
          printf("[BSG_ERROR] Host EPA not mapped\n");

        host_resp_packet.response.data = data;
        // Inject the response packet into manycore response FIFO
        // Pop the request FIFO
        for(int j = 0; j < 4; j++) {
          mc_to_host_resp_fifo->fifo[j].push(host_resp_packet.words[j]);
          host_to_mc_req_fifo->fifo[j].pop();
        }
      }
      else
        printf("[BSG_ERROR] Operations other than loads are not implemented for the host\n");
    }
    else {
      pkt = reinterpret_cast<__m128i *>(&packet);

      // Try to transmit until you actually can
      err = dpi->tx_req(*pkt);

      // Pop the FIFO once transmitted
      if (err == BSG_NONSYNTH_DPI_SUCCESS) {
        for (int i = 0;i < 4; i++)
          host_to_mc_req_fifo->fifo[i].pop();
      }
    }
  }
}

/*
 * dromajo_receive_packet
 * Receives packets from the DPI FIFOs and pushes the data into the
 * manycore request and response FIFOs in Dromajo
 */
void dromajo_receive_packet() {
  hb_mc_packet_t *req_packet, *resp_packet;
  int err;
  __m128i *pkt;

  // Read from the manycore request FIFO
  err = dpi->rx_req(*pkt);
  req_packet = reinterpret_cast<hb_mc_packet_t *>(pkt);

  // Push to host FIFO if valid
  if (err == BSG_NONSYNTH_DPI_SUCCESS) {
    for (int i = 0; i < 4; i++) {
      mc_to_host_req_fifo->fifo[i].push(req_packet->words[i]);
    }
  }

  // Read from the manycore response FIFO
  err = dpi->rx_rsp(*pkt);
  resp_packet = reinterpret_cast<hb_mc_packet_t *>(pkt);

  // Push to host FIFO if valid
  if (err == BSG_NONSYNTH_DPI_SUCCESS) {
    for (int i = 0; i < 4; i++) {
      mc_to_host_resp_fifo->fifo[i].push(resp_packet->words[i]);
    }
  }
}

/*
 * dromajo_set_credits
 * Polls the hardware for credit information and sets the credits info
 * for the Dromajo->Manycore request FIFO in dromajo
 */
void dromajo_set_credits() {
  int credits;
  int res = dpi->get_credits(credits);
  if (res == BSG_NONSYNTH_DPI_SUCCESS) {
    if (credits < 0)
      printf("[BSG_WARN] Warning! Credit value is negative!\n");

    host_to_mc_req_fifo->credits = credits;
  }
}

////////////////////////////// Interacting with VCS //////////////////////////////

/*
 * get_argc
 * Given a string, determine the number of space-separated arguments
 * @params[in] args - Pointer to a character array that holds the arguments
 * @returns the number of arguments
 */
int get_argc(char * args){
        char *cur = args, prev=' ';
        int count = 1;
        while(*cur != '\0'){
                if((prev == ' ') && (prev != *cur)){
                        count ++;
                }
                prev = *cur;
                ++cur;
        }
        return count;
}

/*
 * get_argv
 * Given a string, retrieves the space-separated arguments
 * @params[in] args - Pointer to a character array that holds the arguments
 * @params[in] argc - Number of arguments
 * @params[in] argv - Pointer to an array of strings that will hold the different
 * arguments
 */
static
void get_argv(char * args, int argc, char **argv){
        int count = 0;
        char *cur = args, prev=' ';

        // First parse the path name. This is not in the argument string because
        // VCS doesn't provide it to us. Instead, we "hack" around it by reading
        // the path from 'proc/self/exe'. The maximum path-name length is 1024,
        // with an extra null character for safety
        static char path[1025] = {'\0'};

        readlink("/proc/self/exe", path, sizeof(path) - 1);
        argv[0] = path;
        count ++;

        // Then we parse the remaining arguments. Arguments are separated by N
        // >= 1 spaces. We only register an argument when the previous character
        // was a space, and the current character is not (so that multiple
        // spaces don't count as multiple arguments). We replace spaces with
        // null characters (\0) so that each argument appears to be an
        // individual string and can be used later, by argparse (and other
        // libraries)
        while(*cur != '\0'){
                if((prev == ' ') && (prev != *cur)){
                        argv[count] = cur;
                        count++;
                }
                prev = *cur;
                if(*cur == ' ')
                        *cur = '\0';
                cur++;
        }
}

/*
 * vcs_main
 * The main function for the x86 host. It initializes all devices and controls
 * the time in simulation. It handles all DPI calls and passes packets to and from
 * Dromajo during execution
 * @params[in] argc - Number of arguments
 * @params[in] argv - Pointer to an array of string arguments
 * @returns
 */
int vcs_main(int argc, char **argv) {
  // Push command-line arguments into global variables
  _argc = argc;
  _argv = argv;


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
    printf("[BSG_ERROR] Failed to initialize DPI!\nExiting...\n");
    return HB_MC_FAIL;
  }

  // Wait for reset to be done
  err = dpi_wait_for_reset(dpi);
  if (err != HB_MC_SUCCESS) {
    dpi_clean(dpi);
    delete sim;
    printf("[BSG_ERROR] Failed to wait for reset!\nExiting...\n");
    return HB_MC_FAIL;
  }

  while(1)
    sim->eval();

  return HB_MC_SUCCESS;
}

/*
 * cosim_main
 * This function is the VCS hook for cosimulation
 * @params[in] exit_code - A pointer to an integer that holds the exit code
 * @params[in] args - A character array that holds the space-separated
 * arguments to the function
 */
void cosim_main(uint32_t *exit_code, char *args) {
        // We aren't passed command line arguments directly so we parse them
        // from *args. args is a string from VCS - to pass a string of arguments
        // to args, pass c_args to VCS as follows: +c_args="<space separated
        // list of args>"
        int argc = get_argc(args);
        char *argv[argc];
        get_argv(args, argc, argv);

        int rc = vcs_main(argc, argv);
        *exit_code = rc;
        bsg_pr_test_pass_fail(rc == HB_MC_SUCCESS);
        return;
}
