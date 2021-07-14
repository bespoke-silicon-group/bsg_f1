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

#include <bsg_manycore_platform.h>
#include <bsg_manycore.h>
#include <bsg_manycore_printing.h>
#include <bsg_manycore_config.h>

#include <bp_hb_platform.h>

#include <set>

/* these are convenience macros that are only good for one line prints */
#define manycore_pr_dbg(mc, fmt, ...) \
        bsg_pr_dbg("%s: " fmt, mc->name, ##__VA_ARGS__)

#define manycore_pr_err(mc, fmt, ...) \
        bsg_pr_err("%s: " fmt, mc->name, ##__VA_ARGS__)

#define manycore_pr_warn(mc, fmt, ...) \
        bsg_pr_warn("%s: " fmt, mc->name, ##__VA_ARGS__)

#define manycore_pr_info(mc, fmt, ...) \
        bsg_pr_info("%s: " fmt, mc->name, ##__VA_ARGS__)

/********************************** BlackParrot platform API **********************************/

/*
 * Reads the manycore bridge for number of credits used in the endpoint
 * @param[in] credits_used --> Pointer to a location in memory that will hold the number of credits used
 * @returns HB_MC_SUCCESS
 */
int bp_hb_get_credits_used(int *credits_used) {
  uint32_t *bp_to_mc_req_credits_addr = reinterpret_cast<uint32_t *>(MC_BASE_ADDR + BP_TO_MC_REQ_CREDITS_ADDR);
  *credits_used = (int) (*bp_to_mc_req_credits_addr);
  if (*credits_used < 0) {
    bsg_pr_err("Credits used cannot be negative. Credits used = %d", *credits_used);
    return HB_MC_FAIL;
  }
  return HB_MC_SUCCESS;
}

/*
 * Writes a 128-bit manycore packet in 32-bit chunks to the manycore bridge FIFO
 * @param[in] pkt --> Pointer to the manycore packet
 * @returns HB_MC_SUCCESS
 * TODO: Implement error checking (Requires some modifications in Dromajo)
 */
int bp_hb_write_to_mc_bridge(hb_mc_packet_t *pkt) {
  uint32_t *bp_to_mc_req_fifo_addr = reinterpret_cast<uint32_t *>(MC_BASE_ADDR + BP_TO_MC_REQ_FIFO_ADDR);
  for(int i = 0; i < 4; i++) {
    *bp_to_mc_req_fifo_addr = pkt->words[i];
    bp_to_mc_req_fifo_addr++;
  }
  return HB_MC_SUCCESS;
}

/*
 * Checks if the MC to BP FIFO contains any valid elements to be read
 * @param[in] entries --> Pointer to a location in memory that will hold the number of entries
 * @param[in] type --> Type of FIFO to read from
 * @returns HB_MC_SUCCESS on success or HB_MC_FAIL on fail
 */
int bp_hb_get_fifo_entries(int *entries, hb_mc_fifo_rx_t type) {
  uint32_t *addr;
  switch (type) {
    case HB_MC_FIFO_RX_REQ: addr = reinterpret_cast<uint32_t *>(MC_BASE_ADDR + MC_TO_BP_REQ_ENTRIES_ADDR);
    break;
    case HB_MC_FIFO_RX_RSP: addr = reinterpret_cast<uint32_t *>(MC_BASE_ADDR + MC_TO_BP_RESP_ENTRIES_ADDR);
    break;
    default:
    {
      bsg_pr_err("%s: Unknown packet type\n", __func__);
      return HB_MC_FAIL;
    }
    break;
  }
  
  *entries = *addr;
  if (*entries < 0) {
    bsg_pr_err("Entries occupied cannot be negative. Entries = %d", *entries);
    return HB_MC_FAIL;
  }

  return HB_MC_SUCCESS;
}

/*
 * Reads the manycore bridge FIFOs in 32-bit chunks to form the 128-bit packet
 * @param[in] pkt --> Pointer to the manycore packet
 * @param[in] type --> Type of FIFO to read from
 * @returns HB_MC_SUCCESS on success, HB_MC_FAIL if FIFO type is unknown
 */
int bp_hb_read_from_mc_bridge(hb_mc_packet_t *pkt, hb_mc_fifo_rx_t type) {
  uint32_t *addr;
  switch(type) {
    case HB_MC_FIFO_RX_REQ: addr = reinterpret_cast<uint32_t *>(MC_BASE_ADDR + MC_TO_BP_REQ_FIFO_ADDR);
    break;
    case HB_MC_FIFO_RX_RSP: addr = reinterpret_cast<uint32_t *>(MC_BASE_ADDR + MC_TO_BP_RESP_FIFO_ADDR);
    break;
    default:
    {
      bsg_pr_err("%s: Unknown packet type\n", __func__);
      return HB_MC_FAIL;
    }
    break;
  }

  uint32_t fifo_read_status = DROMAJO_RW_FAIL_CODE;
  do {
    for (int i = 0; i < 4; i++) {
      pkt->words[i] = *addr;
      fifo_read_status &= pkt->words[i];
      addr++;
    }
  } while (fifo_read_status == DROMAJO_RW_FAIL_CODE);

  // There is something wrong if the read status is equal to the FAIL code
  if (fifo_read_status == DROMAJO_RW_FAIL_CODE)
    return HB_MC_FAIL;

  return HB_MC_SUCCESS;
}

/********************************** Manycore platform API **********************************/

typedef struct hb_mc_platform_t
{
  hb_mc_manycore_id_t id;
} hb_mc_platform_t;

static std::set<hb_mc_manycore_id_t> active_ids;

/**
 * Clean up the runtime platform
 * @param[in] mc    A manycore to clean up
 */
void hb_mc_platform_cleanup(hb_mc_manycore_t *mc) {
  hb_mc_platform_t *platform = reinterpret_cast<hb_mc_platform_t *>(mc->platform);

  // Remove the key
  auto key = active_ids.find(platform->id);
  active_ids.erase(key);

  platform->id = 0;

  // Ideally, we would clean up each platform in
  // platforms. However, we can't guarantee that someone won't
  // call init again, so we'll live with the memory leak for
  // now. It's a rare case, since few people call init/cleanup
  // and then continue their program indefinitley.

  return;
}

/**
 * Initialize the runtime platform
 * @param[in] mc    A manycore to initialize
 * @param[in] id    ID which selects the physical hardware from which this manycore is configured
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_platform_init(hb_mc_manycore_t *mc, hb_mc_manycore_id_t id) {
  hb_mc_platform_t *platform = new hb_mc_platform_t;
  
  // check if mc is already initialized
  if (mc->platform)
    return HB_MC_INITIALIZED_TWICE;

  if (id != 0) 
  {
    manycore_pr_err(mc, "Failed to init platform: invalid ID\n");
    return HB_MC_INVALID;
  }

  // Check if the ID has already been initialized
  if(active_ids.find(id) != active_ids.end())
  {
    manycore_pr_err(mc, "Already initialized ID\n");
    return HB_MC_INVALID;
  }

  active_ids.insert(id);
  platform->id = id;

  mc->platform = reinterpret_cast<void *>(platform);

  return HB_MC_SUCCESS;
}

/**
 * Transmit a request packet to manycore hardware
 * @param[in] mc      A manycore instance initialized with hb_mc_manycore_init()
 * @param[in] request A request packet to transmit to manycore hardware
 * @param[in] timeout A timeout counter. Unused - set to -1 to wait forever.
 * @return HB_MC_SUCCESS on success. Otherwise an error code defined in bsg_manycore_errno.h.
 */
int hb_mc_platform_transmit(hb_mc_manycore_t *mc, hb_mc_packet_t *packet, hb_mc_fifo_tx_t type, long timeout) {
  hb_mc_platform_t *platform = reinterpret_cast<hb_mc_platform_t *>(mc->platform);
  const char *typestr = hb_mc_fifo_tx_to_string(type);
  const hb_mc_config_t *cfg = hb_mc_manycore_get_config(mc);
  uint32_t max_credits = hb_mc_config_get_transmit_vacancy_max(cfg);
  int err;

  // Timeout is unsupported
  if (timeout != -1)
  {
    manycore_pr_err(mc, "%s: Only a timeout value of -1 is supported\n", __func__);
    return HB_MC_INVALID;
  }

  // Host doesn't send responses to the mamycore
  if (type == HB_MC_FIFO_TX_RSP)
  {
    manycore_pr_err(mc, "TX Response Not Supported!\n");
    return HB_MC_NOIMPL;
  }

  int credits_used = 0;
  do {
    err = bp_hb_get_credits_used(&credits_used);
  } while ((err != HB_MC_SUCCESS) || ((max_credits - credits_used) <= 0));

  // Don't need to check for error code since this operation should always be a success since it is not going
  // over the network
  err = bp_hb_write_to_mc_bridge(packet);
  if (err != HB_MC_SUCCESS) {
    bsg_pr_err("Write to the host request FIFO failed!");
    return err;
  }

  return HB_MC_SUCCESS;
}

/**
 * Receive a packet from manycore hardware
 * @param[in] mc       A manycore instance initialized with hb_mc_manycore_init()
 * @param[in] response A packet into which data should be read
 * @param[in] timeout  A timeout counter. Unused - set to -1 to wait forever.
 * @return HB_MC_SUCCESS on success. Otherwise an error code defined in bsg_manycore_errno.h.
 */
int hb_mc_platform_receive(hb_mc_manycore_t *mc, hb_mc_packet_t *packet, hb_mc_fifo_rx_t type, long timeout) {
  hb_mc_platform_t *platform = reinterpret_cast<hb_mc_platform_t *>(mc->platform);
  const char *typestr = hb_mc_fifo_rx_to_string(type);

  // Timeout is unsupported
  if (timeout != -1)
  {
    manycore_pr_err(mc, "%s: Only a timeout value of -1 is supported\n", __func__);
    return HB_MC_INVALID;
  }

  int err;
  int num_entries = 0;
  do {
    err = bp_hb_get_fifo_entries(&num_entries, type);
  } while ((num_entries == 0) || (err != HB_MC_SUCCESS));

  err = bp_hb_read_from_mc_bridge(packet, type);
  if (err != HB_MC_SUCCESS) {
    bsg_pr_err("Read from the %s FIFO did not succeed", typestr);
    return HB_MC_INVALID;
  }

  return HB_MC_SUCCESS;
}

/**
 * Read the configuration register at an index
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  idx    Configuration register index to access
 * @param[out] config Configuration value at index
 * @return HB_MC_SUCCESS on success. Otherwise an error code defined in bsg_manycore_errno.h.
 */
int hb_mc_platform_get_config_at(hb_mc_manycore_t *mc, unsigned int idx, hb_mc_config_raw_t *config) {
  hb_mc_platform_t *platform = reinterpret_cast<hb_mc_platform_t *>(mc->platform);
  hb_mc_packet_t config_req_pkt, config_resp_pkt;
  int num_entries = 0;
  int err;

  if (idx < HB_MC_CONFIG_MAX)
  {
    config_req_pkt.request.x_dst = HOST_X_COORD;
    config_req_pkt.request.y_dst = HOST_Y_COORD;
    config_req_pkt.request.x_src = BP_HOST_LINK_X;
    config_req_pkt.request.y_src = BP_HOST_LINK_Y;
    config_req_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_LOAD;
    config_req_pkt.request.payload = 0;
    config_req_pkt.request.reg_id = 0;
    config_req_pkt.request.addr = HB_MC_HOST_EPA_CONFIG_START + idx;

    // Note: Potentially dangerous to write to the FIFO without checking for credits
    // We get back credits used and not credits remaining and without the configuration
    // there is no way to know the credits remaining.
    err = bp_hb_write_to_mc_bridge(&config_req_pkt);
    if (err != HB_MC_SUCCESS) {
      bsg_pr_err("Write to the host request FIFO failed!");
      return err;
    }

    do {
      err = bp_hb_get_fifo_entries(&num_entries, HB_MC_FIFO_RX_RSP);
    } while ((num_entries == 0) || (err != HB_MC_SUCCESS));

    err = bp_hb_read_from_mc_bridge(&config_resp_pkt, HB_MC_FIFO_RX_RSP);
    if (err != HB_MC_SUCCESS) {
      bsg_pr_err("Config read failed\n");
      return HB_MC_FAIL;
    }

    uint32_t data = config_resp_pkt.response.data;
    *config = (data != HB_MC_HOST_OP_FINISH_CODE) ? data : 0;

    return HB_MC_SUCCESS;
  }

  return HB_MC_INVALID;
}

/**
 * Stall until the all requests (and responses) have reached their destination.
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in] timeout A timeout counter. Unused - set to -1 to wait forever.
 * @return HB_MC_SUCCESS on success. Otherwise an error code defined in bsg_manycore_errno.h.
 */
int hb_mc_platform_fence(hb_mc_manycore_t *mc, long timeout) {
  int credits_used, is_vacant, num_entries, err;
  hb_mc_packet_t fence_req_pkt, fence_resp_pkt;
  num_entries = 0;
  credits_used = 0;

  hb_mc_platform_t *platform = reinterpret_cast<hb_mc_platform_t *>(mc->platform); 

  if (timeout != -1) {
    manycore_pr_err(mc, "%s: Only a timeout value of -1 is supported\n", __func__);
    return HB_MC_NOIMPL;
  }

  // Prepare host packet to query TX vacancy
  fence_req_pkt.request.x_dst = HOST_X_COORD;
  fence_req_pkt.request.y_dst = HOST_Y_COORD;
  fence_req_pkt.request.x_src = BP_HOST_LINK_X;
  fence_req_pkt.request.y_src = BP_HOST_LINK_Y;
  fence_req_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_LOAD;
  fence_req_pkt.request.payload = 0;
  fence_req_pkt.request.reg_id = 0;
  fence_req_pkt.request.addr = HB_MC_HOST_EPA_TX_VACANT; // x86 Host address to poll tx vacancy

  do
  {
    err = bp_hb_get_credits_used(&credits_used);
    // In a real system, this function call makes no sense since we will be sending packets to the
    // host on the network and are trying to check for credits to be zero and complete the fence.
    // It is fine here because of the system setup.
    err |= bp_hb_write_to_mc_bridge(&fence_req_pkt);

    do {
      err = bp_hb_get_fifo_entries(&num_entries, HB_MC_FIFO_RX_RSP);
    } while ((num_entries == 0) || (err != HB_MC_SUCCESS));

    err |= bp_hb_read_from_mc_bridge(&fence_resp_pkt, HB_MC_FIFO_RX_RSP);
    is_vacant = fence_resp_pkt.response.data;
  } while ((err == HB_MC_SUCCESS) && !((credits_used == 0) && is_vacant));

  return err;
}

/**
 * Signal the hardware to start a bulk transfer over the network
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @return HB_MC_SUCCESS on success. Otherwise an error code defined in bsg_manycore_errno.h.
 */
int hb_mc_platform_start_bulk_transfer(hb_mc_manycore_t *mc) {
  return HB_MC_SUCCESS;
}

/**
 * Signal the hardware to end a bulk transfer over the network
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @return HB_MC_SUCCESS on success. Otherwise an error code defined in bsg_manycore_errno.h.
 */
int hb_mc_platform_finish_bulk_transfer(hb_mc_manycore_t *mc) {
  return HB_MC_SUCCESS;
}

/**
 * Get the current cycle counter of the Manycore Platform
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[out] time   The current counter value.
 * @return HB_MC_SUCCESS on success. Otherwise an error code defined in bsg_manycore_errno.h.
 */
int hb_mc_platform_get_cycle(hb_mc_manycore_t *mc, uint64_t *time) {
  return HB_MC_NOIMPL;
}

/**
 * Get the number of instructions executed for a certain class of instructions
 * @param[in] mc    A manycore instance initialized with hb_mc_manycore_init()
 * @param[in] itype An enum defining the class of instructions to query.
 * @param[out] count The number of instructions executed in the queried class.
 * @return HB_MC_SUCCESS on success. Otherwise an error code defined in bsg_manycore_errno.h.
 */
int hb_mc_platform_get_icount(hb_mc_manycore_t *mc, bsg_instr_type_e itype, int *count) {
  return HB_MC_NOIMPL;
}

/**
 * Enable trace file generation (vanilla_operation_trace.csv)
 * @param[in] mc    A manycore instance initialized with hb_mc_manycore_init()
 * @return HB_MC_SUCCESS on success. Otherwise an error code defined in bsg_manycore_errno.h.
 */
int hb_mc_platform_trace_enable(hb_mc_manycore_t *mc) {
  return HB_MC_NOIMPL;
}

/**
 * Disable trace file generation (vanilla_operation_trace.csv)
 * @param[in] mc    A manycore instance initialized with hb_mc_manycore_init()
 * @return HB_MC_SUCCESS on success. Otherwise an error code defined in bsg_manycore_errno.h.
 */
int hb_mc_platform_trace_disable(hb_mc_manycore_t *mc) {
  return HB_MC_NOIMPL;
}

/**
 * Enable log file generation (vanilla.log)
 * @param[in] mc    A manycore instance initialized with hb_mc_manycore_init()
 * @return HB_MC_SUCCESS on success. Otherwise an error code defined in bsg_manycore_errno.h.
 */
int hb_mc_platform_log_enable(hb_mc_manycore_t *mc) {
  return HB_MC_NOIMPL;
}

/**
 * Disable log file generation (vanilla.log)
 * @param[in] mc    A manycore instance initialized with hb_mc_manycore_init()
 * @return HB_MC_SUCCESS on success. Otherwise an error code defined in bsg_manycore_errno.h.
 */
int hb_mc_platform_log_disable(hb_mc_manycore_t *mc) {
  return HB_MC_NOIMPL;
}

/**
 * Block until chip reset has completed.
 * @param[in] mc    A manycore instance initialized with hb_mc_manycore_init()
 * @return HB_MC_SUCCESS on success. Otherwise an error code defined in bsg_manycore_errno.h.
 */        
int hb_mc_platform_wait_reset_done(hb_mc_manycore_t *mc) {
  hb_mc_packet_t reset_req_pkt, reset_resp_pkt;
  int err;
  uint32_t data = 0;

  reset_req_pkt.request.x_dst = HOST_X_COORD;
  reset_req_pkt.request.y_dst = HOST_Y_COORD;
  reset_req_pkt.request.x_src = BP_HOST_LINK_X;
  reset_req_pkt.request.y_src = BP_HOST_LINK_Y;
  reset_req_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_LOAD;
  reset_req_pkt.request.payload = 0;
  reset_req_pkt.request.reg_id = 0;
  reset_req_pkt.request.addr = HB_MC_HOST_EPA_RESET_DONE;

  do {
    // The platform setup ensures that this packet will not go over the network so
    // we don't need to check for credits.
    err = bp_hb_write_to_mc_bridge(&reset_req_pkt);
    err |= bp_hb_read_from_mc_bridge(&reset_resp_pkt, HB_MC_FIFO_RX_RSP);

    data = reset_resp_pkt.response.data;
  } while((err != HB_MC_SUCCESS) || (data == 0));

  return HB_MC_SUCCESS;
}