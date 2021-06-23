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
#include <svdpi.h>
// We use __m128i so that we can pass a 128-bit type between Verilog
// and C.
#include <xmmintrin.h>

// DPI function to advance time
extern "C" {
  void bsg_dpi_next();
}

// Global variables to store arguments
int _argc;
char **_argv;
static int char_index = 0;

////////////////////////////// SimulationWrapper functions //////////////////////////////

// Constructor
SimulationWrapper::SimulationWrapper(){
  root = new std::string("replicant_tb_top");
  std::string mc_dpi = *root + ".mc_dpi";
  top = svGetScopeFromName(mc_dpi.c_str());
  dpi = new bsg_nonsynth_dpi::dpi_manycore<HB_MC_CONFIG_MAX>(mc_dpi);

  // Initialize DPI and Dromajo
  dromajo_init();
  dpi_init();

  if ((!dromajo) || (!dpi)) {
    bsg_pr_err("Failed to initialize DPI or Dromajo pointer\n");
  }
}

// Destructor
SimulationWrapper::~SimulationWrapper(){
  dpi_cleanup();
  dromajo_cosim_fini(dromajo);
  this->top = nullptr;
}

// Causes time to proceed by 1 unit
void SimulationWrapper::advance_time() {
  svScope prev;
  prev = svSetScope(top);
  bsg_dpi_next();
  svSetScope(prev);
}

// Does nothing. Turning on/off assertions is only supported in
// Verilator.
void SimulationWrapper::assertOn(bool val){

}

std::string SimulationWrapper::getRoot(){
  return *root;
}

/*
 * dpi_cleanup
 * Destroys the pointer to the manycore DPI object
 * @params[in] dpi - Pointer to the manycore DPI object
 */
void SimulationWrapper::dpi_cleanup() {
  delete dpi;
}

/*
 * dpi_fifo_drain
 * Drains all DPI fifos. This function is used during initialization and does not push packets
 * to the FIFOs in Dromajo
 * @param[in] dpi - Pointer to the manycore DPI object
 * @param[in] type - Manycore FIFO type
 * @returns success if all the FIFOs were drained correctly
 */
int SimulationWrapper::dpi_fifo_drain(hb_mc_fifo_rx_t type) {
  __m128i *pkt;

  int err, drains = 0;
  int cap = dpi->config[HB_MC_CONFIG_IO_REMOTE_LOAD_CAP];

  do {
    advance_time();

    switch(type) {
      case HB_MC_FIFO_RX_REQ:
        err = dpi->rx_req(*pkt);
      break;
      case HB_MC_FIFO_RX_RSP:
        err = dpi->rx_rsp(*pkt);
      break;
      default:
        bsg_pr_err("Unknown FIFO type\n");
        return HB_MC_FAIL;
    }

    if (err == BSG_NONSYNTH_DPI_SUCCESS)
      drains++;
  } while ((err == BSG_NONSYNTH_DPI_NOT_WINDOW || err == BSG_NONSYNTH_DPI_SUCCESS)
          && drains <= cap);

  if (drains == cap) {
    bsg_pr_err("Failed to drain FIFO\n");
    return HB_MC_FAIL;
  }

  return HB_MC_SUCCESS;
}

/*
 * dpi_init
 * Initializes the DPI between the manycore and Dromajo
 * @params[in] dpi - Pointer to the manycore DPI object
 * @params[in] hierarchy - A C++ string that holds the manycore hierarchy
 * @returns success if initialized correctly
 */
int SimulationWrapper::dpi_init() {
  advance_time();

  int err;

  err = dpi_fifo_drain(HB_MC_FIFO_RX_REQ);
  if (err != HB_MC_SUCCESS) {
    dpi_cleanup();
    return err;
  }

  err = dpi_fifo_drain(HB_MC_FIFO_RX_RSP);
  if (err != HB_MC_SUCCESS) {
    dpi_cleanup();
    return err;
  }

  return HB_MC_SUCCESS;
}

/*
 * dromajo_init
 * Initializes dromajo with the correct command-line arguments
 * @returns a pointer to a dromajo_cosim_state_t object
 */
int SimulationWrapper::dromajo_init() {
  char dromajo_str[50];
  char host_str[50];
  char manycore_str[50];
  char prog_str[50];

  sprintf(dromajo_str, "dromajo");
  sprintf(host_str, "--host");
  sprintf(manycore_str, "--manycore");
  sprintf(prog_str, "main.elf");

  char* argv[] = {dromajo_str, host_str, manycore_str, prog_str};
  dromajo = dromajo_cosim_init(4, argv);
  if (!dromajo) {
    bsg_pr_err("Failed to initialize Dromajo!\n");
    return HB_MC_FAIL;
  }
  return HB_MC_SUCCESS;
}

/*
 * dromajo_step
 * Executes 1 instruction in Dromajo
 * @returns true if execution is incomplete and/or without errors
 */
bool SimulationWrapper::dromajo_step() {
  // Execute dromajo with verbose mode on
  int err = dromajo_cosim_step(dromajo, 0, 0, 0, 0, 0, false, false);
  return (err != 0) ? false : true;
}

/*
 * dromajo_transmit_packet
 * Fetches data from the Dromajo->Manycore FIFO and pushes it into the DPI FIFO
 * to send packets to the manycore
 * @returns success on succesful transmission
 */
int SimulationWrapper::dromajo_transmit_packet() {
  hb_mc_packet_t dromajo_to_mc_packet, host_to_dromajo_packet;
  int err;
  __m128i *pkt;

  // Check if FIFO has an element and hence ready to transmit
  mc_fifo_type_t type = FIFO_HOST_TO_MC_REQ;
  bool is_empty = mc_is_fifo_empty(type);

  if (!is_empty) {
    // Read the FIFO head pointer for all 32-bit FIFOs
    dromajo_to_mc_packet.words[0] = host_to_mc_req_fifo->fifo[0].front();
    dromajo_to_mc_packet.words[1] = host_to_mc_req_fifo->fifo[1].front();
    dromajo_to_mc_packet.words[2] = host_to_mc_req_fifo->fifo[2].front();
    dromajo_to_mc_packet.words[3] = host_to_mc_req_fifo->fifo[3].front();

    // Intercept packets that are for the host and generate appropriate responses
    if ((dromajo_to_mc_packet.request.x_dst == ((0 << 4) | 0)) && (dromajo_to_mc_packet.request.y_dst == ((1 << 3) | 0))) {
      host_to_dromajo_packet.response.x_dst = dromajo_to_mc_packet.request.x_src;
      host_to_dromajo_packet.response.y_dst = dromajo_to_mc_packet.request.y_src;
      host_to_dromajo_packet.response.load_id = 0;

      uint32_t data = 0;
      switch (dromajo_to_mc_packet.request.op_v2) {
        case HB_MC_PACKET_OP_REMOTE_LOAD:
        {
          // Fixme: Is there no struct for response opcode like in the manycore hardware
          host_to_dromajo_packet.response.op = dromajo_to_mc_packet.request.op_v2;
          switch (dromajo_to_mc_packet.request.addr) {
            case MC_ARGS_START_EPA_ADDR ... MC_ARGS_FINISH_EPA_ADDR:
            {
              uint32_t idx = dromajo_to_mc_packet.request.addr - MC_ARGS_START_EPA_ADDR;
              int num_characters = 0;
              // If all arguments have been read or there are no arguments to read
              // send a finish code
              if ((idx != _argc) && (_argc != 0)) {
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
              else
                data = MC_HOST_OP_FINISH_CODE;
            }
            break;
            case MC_CONFIG_START_EPA_ADDR ... MC_CONFIG_FINISH_EPA_ADDR:
            {
              uint32_t idx = dromajo_to_mc_packet.request.addr - MC_CONFIG_START_EPA_ADDR;
              data = (idx <= HB_MC_CONFIG_MAX) ? dpi->config[idx] : MC_HOST_OP_FINISH_CODE;
            }
            break;
            case MC_RESET_DONE_EPA_ADDR:
            {
              bool done;
              dpi->reset_is_done(done);
              data = done ? 1 : 0;
            }
            break;
            case MC_TX_VACANT_EPA_ADDR:
            {
              bool is_vacant;
              dpi->tx_is_vacant(is_vacant);
              data = is_vacant ? 1 : 0;
            }
            break;
            default:
            {
              bsg_pr_err("Invalid address for host load operation\n");
              return HB_MC_FAIL;
            }

          }
          host_to_dromajo_packet.response.data = data;
          for(int j = 0; j < 4; j++) {
            mc_to_host_resp_fifo->fifo[j].push(host_to_dromajo_packet.words[j]);
            host_to_mc_req_fifo->fifo[j].pop();
          }
        }
        break;
        case HB_MC_PACKET_OP_REMOTE_SW:
        {
          switch (dromajo_to_mc_packet.request.addr) {
            case MC_STDOUT_EPA_ADDR:
            case MC_STDERR_EPA_ADDR:
              printf("%c", (uint8_t) dromajo_to_mc_packet.request.payload);
            break;
            case MC_FINISH_EPA_ADDR:
            {
              int16_t core_id = dromajo_to_mc_packet.request.payload >> 16;
              // Success error codes in BP is 0, but 0 is already used by the manycore. Any positive number indicates
              // success, therefore, add 1.
              int err = (0x0000FFFF & dromajo_to_mc_packet.request.payload) + 1;
              bsg_pr_info("Core %d successfully terminated\n", core_id);
              // De-allocate all pointers prior to termination
              dpi_cleanup();
              dromajo_cosim_fini(dromajo);
              return err;
            }
            break;
            case MC_FAIL_EPA_ADDR:
            {
              int16_t core_id = dromajo_to_mc_packet.request.payload >> 16;
              int16_t err = 0x0000FFFF & dromajo_to_mc_packet.request.payload;
              bsg_pr_err("Core %d terminated with code %d\n", core_id, err);
              // De-allocate all pointers prior to termination
              dpi_cleanup();
              dromajo_cosim_fini(dromajo);
              return err;
            }
            break;
            default:
            {
              bsg_pr_err("Invalid address for host store operation\n");
              return HB_MC_FAIL;
            }
          }
          // Store requests don't have responses so only pop the host to MC fifo
          for(int j = 0; j < 4; j++) {
            host_to_mc_req_fifo->fifo[j].pop();
          }
        }
        break;
        default:
        {
          bsg_pr_err("Operations other than loads and store words are not implemented for the host\n");
          return HB_MC_FAIL;
        }
        break;
      }
    }
    else {
      pkt = reinterpret_cast<__m128i *>(&dromajo_to_mc_packet);

      // Allows the DPI interface to track response FIFO capacity
      bool expect_response =
        (dromajo_to_mc_packet.request.op_v2 != HB_MC_PACKET_OP_REMOTE_STORE) &&
        (dromajo_to_mc_packet.request.op_v2 != HB_MC_PACKET_OP_REMOTE_SW) &&
        (dromajo_to_mc_packet.request.op_v2 != HB_MC_PACKET_OP_CACHE_OP);

      // Attempt packet transmission
      // Since we trigger a call to the transmit FIFOs only when the Dromajo
      // FIFOs are full, we need to wait until the DPI FIFOs are ready to receive
      // before advancing to the next operation. This can prevent filling up of the
      // FIFOs. However, not doing this can help in identifying situations that might
      // create backpressure in actual hardware and provision for it.
      do {
        advance_time();
        err = dpi->tx_req(*pkt, expect_response);
      } while (err != BSG_NONSYNTH_DPI_SUCCESS &&
          (err == BSG_NONSYNTH_DPI_NO_CREDITS ||
           err == BSG_NONSYNTH_DPI_NO_CAPACITY ||
           err == BSG_NONSYNTH_DPI_NOT_WINDOW ||
           err == BSG_NONSYNTH_DPI_BUSY       ||
           err == BSG_NONSYNTH_DPI_NOT_READY));

      if (err == BSG_NONSYNTH_DPI_SUCCESS) {
        for (int i = 0;i < 4; i++)
          host_to_mc_req_fifo->fifo[i].pop();
      }
      else {
        bsg_pr_err("Packet transmission failed\n");
        return HB_MC_FAIL;
      }

      // Update the credits in dromajo
      bsg_pr_dbg("Checking for credits\n");
      err = dromajo_set_credits();
      if (err != HB_MC_SUCCESS)
        return err;
    }
  }

  return HB_MC_SUCCESS;
}

/*
 * dromajo_receive_packet
 * Receives packets from the DPI FIFOs and pushes the data into the
 * manycore request and response FIFOs in Dromajo
 */
int SimulationWrapper::dromajo_receive_packet() {
  hb_mc_packet_t *mc_to_dromajo_req_packet, *mc_to_dromajo_resp_packet;
  int err;
  __m128i pkt;

  // Read from the manycore request FIFO
  // At every time step we are polling the request FIFO to see if there
  // is a packet. If there is no packet (i.e err == BSG_NONSYNTH_NOT_VALID)
  // we must move on
  do {
    advance_time();
    err = dpi->rx_req(pkt);
  } while (err != BSG_NONSYNTH_DPI_SUCCESS    &&
          (err == BSG_NONSYNTH_DPI_NOT_WINDOW ||
           err == BSG_NONSYNTH_DPI_BUSY       ||
           err == BSG_NONSYNTH_DPI_INVALID));

  if (err == BSG_NONSYNTH_DPI_SUCCESS) {
    mc_to_dromajo_req_packet = reinterpret_cast<hb_mc_packet_t *>(&pkt);
    for (int i = 0; i < 4; i++) {
      mc_to_host_req_fifo->fifo[i].push(mc_to_dromajo_req_packet->words[i]);
    }
  }
  else if (err != BSG_NONSYNTH_DPI_NOT_VALID){
    bsg_pr_err("Failed to receive manycore request packet");
    return HB_MC_FAIL;
  }

  // Read from the manycore response FIFO
  do {
    advance_time();
    err = dpi->rx_rsp(pkt);
  } while (err != BSG_NONSYNTH_DPI_SUCCESS    &&
          (err == BSG_NONSYNTH_DPI_NOT_WINDOW ||
           err == BSG_NONSYNTH_DPI_BUSY       ||
           err == BSG_NONSYNTH_DPI_INVALID));

  if (err == BSG_NONSYNTH_DPI_SUCCESS) {
    mc_to_dromajo_resp_packet = reinterpret_cast<hb_mc_packet_t *>(&pkt);
    for (int i = 0; i < 4; i++) {
      mc_to_host_resp_fifo->fifo[i].push(mc_to_dromajo_resp_packet->words[i]);
    }
  }
  else if (err != BSG_NONSYNTH_DPI_NOT_VALID) {
    bsg_pr_err("Failed to receive manycore response packet");
    return HB_MC_FAIL;
  }

  return HB_MC_SUCCESS;
}

/*
 * dromajo_set_credits
 * Polls the hardware for credit information and sets the credits info
 * for the Dromajo->Manycore request FIFO in dromajo
 */
int SimulationWrapper::dromajo_set_credits() {
  int credits, credits_used, max_credits;
  int err;

  err = dpi->get_credits_used(credits_used);
  if (err != BSG_NONSYNTH_DPI_SUCCESS) {
    bsg_pr_err(bsg_nonsynth_dpi_strerror(err));
    return HB_MC_FAIL;
  }

  err = dpi->get_credits_max(max_credits);
  if (err == BSG_NONSYNTH_DPI_SUCCESS) {
    credits = max_credits - credits_used;
    if (credits < 0)
      bsg_pr_err("Credit value is < 0. Must be non-negative\n");
    host_to_mc_req_fifo->credits = credits;
  }
  else {
    bsg_pr_err(bsg_nonsynth_dpi_strerror(err));
    return err;
  }

  return HB_MC_SUCCESS;
}

int SimulationWrapper::eval(){
  // Execute 100 instructions on Dromajo
  for(int i = 0; i < NUM_DROMAJO_INSTR_PER_TICK; i++) {
    if (!dromajo_step()) {
      // Fixme: Dromajo could also terminate due to premature errors.
      // Use the test outputs to detect PASS/FAIL
      bsg_pr_info("Dromajo Execution Complete!\nExiting...\n");
      dromajo_cosim_fini(dromajo);
      return HB_MC_SUCCESS;
    }

    int err;
    // Poll for packets to be transmitted
    bsg_pr_dbg("Checking for packets to transmit\n");
    err = dromajo_transmit_packet();
    if (err != HB_MC_SUCCESS)
      return err;

    // Poll for packets to be received
    bsg_pr_dbg("Checking for packets to receive\n");
    err = dromajo_receive_packet();
    if (err != HB_MC_SUCCESS)
      return err;
  }

  // Advance time 1 unit
  advance_time();
  return HB_MC_SUCCESS;
}

////////////////////////////// Interacting with VCS //////////////////////////////

/*
 * get_argc
 * Given a string, determine the number of space-separated arguments
 * @params[in] args - Pointer to a character array that holds the arguments
 * @returns the number of arguments
 */
static int get_argc(char * args){
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
static void get_argv(char * args, int argc, char **argv){
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

  // Initialize Host
  SimulationWrapper *host = new SimulationWrapper();
  if (!host) {
    bsg_pr_err("Could not initialize host!\n");
    return HB_MC_FAIL;
  }

  int err;
  do {
    err = host->eval();
    // Codes greater than 0 can be used to terminate a program
    if (err > 0) break;
  } while (err >= 0);

  return err;
}

/*
 * cosim_main
 * This function is the VCS hook for cosimulation
 * @params[in] exit_code - A pointer to an integer that holds the exit code
 * @params[in] args - A character array that holds the space-separated
 * arguments to the function
 */
extern "C" {
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
          bsg_pr_test_pass_fail(rc >= HB_MC_SUCCESS);
          return;
  }
}
