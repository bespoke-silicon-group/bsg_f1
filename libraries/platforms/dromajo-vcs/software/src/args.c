/*
 * args.c
 * This file includes routines that queries the arguments from the x86 host and 
 * loads them into corresponding registers so that BlackParrot can access them
 */

#include <bp_utils.h>
#include <bsg_manycore.h>
#include <bsg_manycore_packet.h>
#include <bsg_manycore_printing.h>
#include <stdio.h>

// For now keep a limit on the number of arguments and the length of the argument
// string.
// TODO: Make this a pointer to a location on the stack
int _argc;
char **_argv;
static int arg_index = 0;
static int char_index = -1;

void __init_args(void) {
  hb_mc_packet_t req_pkt, resp_pkt;

  // Flat buffer to capture all the arguments contiguously
  char buffer[3000];
  char *bufptr = buffer;
  
  // Create a packet for the x86 host
  // Host is at (0, 0)
  // BlackParrot FIFOs are at (0, 1) 
  req_pkt.request.x_dst = 0;
  req_pkt.request.y_dst = 0;
  req_pkt.request.x_src = 0;
  req_pkt.request.y_src = 1;
  req_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_LOAD;
  req_pkt.request.payload = 0;
  req_pkt.request.reg_id = 0;

  while(1) {
    // Word address 0x00 to 0xFF tell the host to retrieve arguments
    req_pkt.request.addr = arg_index;
    
    // Send packet
    bp_hb_write_to_manycore_bridge(&req_pkt);

    // Wait for response
    int err = bp_hb_read_from_manycore_bridge(&resp_pkt, HB_MC_FIFO_RX_RSP);
    if (err != 0) {
      bp_finish(1);
    }

    uint32_t data = resp_pkt.response.data;
    if (data == 0xFFFFFFFF) {
      bsg_pr_info("Argument retrieval complete!\n");
      break;
    }
    else {
      uint32_t mask = 0xFF000000;
      uint8_t byte;
      for(int i = 0; i < 4; i++) {
        byte = data & mask;
        if (byte != 0x0) {
          buffer[char_index] = byte;
          char_index++;
        }
        else {
          buffer[char_index] = '\0';
          arg_index++;
          char_index++;
          break;
        }
        mask = mask >> 8;
      }
    }
  }
  _argc = arg_index;

  // Convert a flat buffer into a 2-D array of pointers as argparse
  // expects it to be
  char *args[_argc];
  char c = '\0';
  int count = 0;
  while (count != _argc) {
    if ((c == '\0') && (c != *bufptr)) {
      args[count] = bufptr;
      count++;
    }
    c = *bufptr;
    bufptr++;
  }
  _argv = args;

}
