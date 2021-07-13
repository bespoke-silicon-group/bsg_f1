/*
 * args.c
 * This file includes routines that queries the arguments from the x86 host and 
 * loads them into corresponding registers so that BlackParrot can access them
 */

#include <bp_hb_platform.h>
#include <bsg_manycore.h>
#include <bsg_manycore_packet.h>
#include <stdio.h>

// Number of arguments that can be retrieved is currently based on the size of the buffer.
// Other ideas (TODO):
// 1. Create pointers to locations of arguments on the stack (traditional method)
// 2. Flush the argument buffer periodically
int _argc;
char **_argv;
static int arg_index = 0;
static int char_index = 0;

/**
 * Retrieve arguments from the manycore host
 */
void __init_args(void) {
  int err;
  hb_mc_packet_t args_req_pkt, args_resp_pkt;

  // Flat buffer to capture all the arguments contiguously
  char buffer[3000];
  char *bufptr = buffer;
  
  // Create a packet for the x86 host
  // BlackParrot FIFO interface is at POD (X, Y) = (0, 1), SUBCOORD (X, Y) = (0, 1)
  args_req_pkt.request.x_dst = HOST_X_COORD;
  args_req_pkt.request.y_dst = HOST_Y_COORD;
  args_req_pkt.request.x_src = (0 << 4) | 0;
  args_req_pkt.request.y_src = (1 << 3) | 1;
  args_req_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_LOAD;
  args_req_pkt.request.payload = 0;
  args_req_pkt.request.reg_id = 0;

  while(1) {
    // The platform setup in simulation ensures that this packet will not go over the network so
    // we don't need to check for credits. Also, this code is executed before the CUDA-lite
    // program starts.
    args_req_pkt.request.addr = HB_MC_HOST_EPA_ARGS_START + arg_index;
    err = bp_hb_write_to_mc_bridge(&args_req_pkt);
    err = bp_hb_read_from_mc_bridge(&args_resp_pkt, HB_MC_FIFO_RX_RSP);
    if (err != HB_MC_SUCCESS)
      bp_finish(err);

    uint32_t data = args_resp_pkt.response.data;
    if (data != HB_MC_HOST_OP_FINISH_CODE) {
      uint32_t mask = 0xFF000000;
      uint8_t byte;
      for(int i = 0; i < 4; i++) {
        byte = data & mask;
        if (byte != (uint8_t)'\0')
          buffer[char_index] = byte;
        else {
          buffer[char_index] = '\0';
          arg_index++;
          break;
        }
        char_index++;
        mask = mask >> 8;
      }
    }
    else
      break;
  }
  _argc = arg_index;

  // Convert a flat buffer into a 2-D array of pointers as argparse
  // expects it to be
  char *args[_argc];
  char c = '\0';
  int count = 0;
  if (_argc != 0) {
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
}
