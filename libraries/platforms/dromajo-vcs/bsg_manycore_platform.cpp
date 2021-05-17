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

#include <set>
#include <bp_utils.h>

/* these are convenience macros that are only good for one line prints */
#define manycore_pr_dbg(mc, fmt, ...) \
        bsg_pr_dbg("%s: " fmt, mc->name, ##__VA_ARGS__)

#define manycore_pr_err(mc, fmt, ...) \
        bsg_pr_err("%s: " fmt, mc->name, ##__VA_ARGS__)

#define manycore_pr_warn(mc, fmt, ...) \
        bsg_pr_warn("%s: " fmt, mc->name, ##__VA_ARGS__)

#define manycore_pr_info(mc, fmt, ...) \
        bsg_pr_info("%s: " fmt, mc->name, ##__VA_ARGS__)

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

  // Write packet to the manycore bridge
  bp_hb_write_to_manycore_bridge(packet);

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

  // Timeout is unsupported
  if (timeout != -1)
  {
    manycore_pr_err(mc, "%s: Only a timeout value of -1 is supported\n", __func__);
    return HB_MC_INVALID;
  }

  int err = bp_hb_read_from_manycore_bridge(packet, type);

  if (err != 0)
  {
    manycore_pr_err(mc, "%s: Failed to receive packet\n", __func__);
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
  hb_mc_packet_t req_pkt, resp_pkt;

  if (idx < HB_MC_CONFIG_MAX)
  {
    req_pkt.request.x_dst = 0;
    req_pkt.request.y_dst = 0;
    req_pkt.request.x_src = 0;
    req_pkt.request.y_src = 1;
    req_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_LOAD;
    req_pkt.request.payload = 0;
    req_pkt.request.reg_id = 0;
    req_pkt.request.addr = 0x100 + idx;
    bp_hb_write_to_manycore_bridge(&req_pkt);
    int err = bp_hb_read_from_manycore_bridge(&resp_pkt, HB_MC_FIFO_RX_RSP);
    if (err != HB_MC_SUCCESS) {
      bsg_pr_err("Config read failed\n");
      return HB_MC_FAIL;
    }

    uint32_t data = resp_pkt.response.data;
    if (data != 0xFFFFFFFF)
      *config = data;
    else
      *config = 0;

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
  int credits, err;
  uint32_t max_credits;
  int is_vacant;
  hb_mc_packet_t req_pkt, resp_pkt;

  const hb_mc_config_t *cfg = hb_mc_manycore_get_config(mc);
  hb_mc_platform_t *platform = reinterpret_cast<hb_mc_platform_t *>(mc->platform); 
  max_credits = hb_mc_config_get_io_endpoint_max_out_credits(cfg);

  if (timeout != -1) {
    manycore_pr_err(mc, "%s: Only a timeout value of -1 is supported\n", __func__);
    return HB_MC_NOIMPL;
  }

  // Prepare host packet to query TX vacancy
  req_pkt.request.x_dst = 0;
  req_pkt.request.y_dst = 0;
  req_pkt.request.x_src = 0;
  req_pkt.request.y_src = 1;
  req_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_LOAD;
  req_pkt.request.payload = 0;
  req_pkt.request.reg_id = 0;
  req_pkt.request.addr = 0x300; // x86 Host address to poll tx vacancy

  do
  {
    credits = bp_hb_get_credits();
    bp_hb_write_to_manycore_bridge(&req_pkt);
    err = bp_hb_read_from_manycore_bridge(&resp_pkt, HB_MC_FIFO_RX_RSP);
    is_vacant = resp_pkt.response.data;
  } while ((err == HB_MC_SUCCESS) && !(credits == max_credits) && (is_vacant == 1));

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
  hb_mc_packet_t req_pkt, resp_pkt;
  int err;
  uint32_t data;

  req_pkt.request.x_src = 0;
  req_pkt.request.y_src = 1;
  req_pkt.request.x_dst = 0;
  req_pkt.request.y_dst = 0;
  req_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_LOAD;
  req_pkt.request.payload = 0;
  req_pkt.request.reg_id = 0;
  req_pkt.request.addr = 0x200;

  do {
    bp_hb_write_to_manycore_bridge(&req_pkt);
    err = bp_hb_read_from_manycore_bridge(&resp_pkt, HB_MC_FIFO_RX_RSP);
    data = resp_pkt.response.data;
  } while((err != HB_MC_SUCCESS) && (data != 1));

  if (data == 0)
    return HB_MC_FAIL;

  return HB_MC_SUCCESS;
}
