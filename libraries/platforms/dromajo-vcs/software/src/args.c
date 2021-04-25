/*
 * args.c
 * This file includes routines that queries the arguments from the x86 host and 
 * loads them into corresponding registers so that BlackParrot can access them
 * The chain of events are:
 *    1. VCS gets arguments from the +c_args options
 *    2. The x86 simulator/host (bsg_manycore_simulator.cpp) accesses them via vcs_main()
 *    3. A subset of these arguments are stored as arrays in the host
 *    4. BlackParrot sends packets to load from this array and stores it in a global array
 *    5. BlackParrot's start code loads the corresponding registers with the arguments
 */

#include <bp_utils.h>
#include <bsg_manycore.h>
#include <bsg_manycore_packet.h>
#include <stdio.h>

int _argc;
char* _argv[10];

void __init_args(void) {
  hb_mc_packet_t req_pkt;
  hb_mc_packet_t *resp_pkt;
  int count = 0;
  
  // Create a packet for the x86 host
  // Host is at (0, 0)
  req_pkt.request.x_dst = 0;
  req_pkt.request.y_dst = 0;
  // BlackParrot FIFOs are at (0, 1)
  req_pkt.request.x_src = 0;
  req_pkt.request.y_src = 1;
  // Use addr 0 and upwards (in increments of 4 bytes)
  // to talk to the x86 host for addresses
  req_pkt.request.addr = 0;
  req_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_LOAD;
  req_pkt.request.payload = 0;
  req_pkt.request.reg_id = 0;

  while(1) {
    // Send packet
    bp_hb_write_to_manycore_bridge(&req_pkt);

    // Wait for response
    int err = bp_hb_read_from_manycore_bridge(resp_pkt, HB_MC_FIFO_RX_RSP);
    if (err != 0) {
      bp_finish(1);
    }

    if (resp_pkt->response.data == 0xFFFFFFFF)
      break;
    else {
      sprintf(_argv[count], "%d", resp_pkt->response.data);
      count++;
    }
    req_pkt.request.addr += 4;
  }
  _argc = count;  
}
