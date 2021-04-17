#include <bsg_manycore.h>
#include <bsg_mem_dma.hpp>
#include <bsg_manycore_vcache.h>
#include <bsg_manycore_printing.h>
#include <bsg_manycore_config_pod.h>
/* these are convenience macros that are only good for one line prints */
#define dma_pr_dbg(mc, fmt, ...)                   \
        bsg_pr_dbg("%s: " fmt, mc->name, ##__VA_ARGS__)

#define dma_pr_err(mc, fmt, ...)                   \
        bsg_pr_err("%s: " fmt, mc->name, ##__VA_ARGS__)

#define dma_pr_warn(mc, fmt, ...)                          \
        bsg_pr_warn("%s: " fmt, mc->name, ##__VA_ARGS__)

#define dma_pr_info(mc, fmt, ...)                          \
        bsg_pr_info("%s: " fmt, mc->name, ##__VA_ARGS__)

using namespace bsg_mem_dma;

static parameter_t *cache_id_to_memory_id;
static parameter_t *cache_id_to_bank_id;
/**
 * Initializes a specialized DRAM bank to channel map for the BigBlade Chip
 */
static
int hb_mc_dma_init_pod_X4Y4_X16_hbm(hb_mc_manycore_t *mc)
{
        hb_mc_coordinate_t pod;
        static parameter_t pod_in_quad_base [2][2] = {
                /* Y/X     0   1  */
                /* 0 */  { 0,  2 },
                /* 1 */  { 4,  6 },
        };

        unsigned long caches_per_channel =
                hb_mc_vcache_num_caches(mc) /
                hb_mc_config_get_dram_channels(&mc->config);

        hb_mc_config_foreach_pod(pod, &mc->config)
        {
                hb_mc_coordinate_t quad =
                        hb_mc_coordinate(pod.x/2,pod.y/2);

                hb_mc_coordinate_t pod_in_quad =
                        hb_mc_coordinate(pod.x%2,pod.y%2);

                parameter_t memory_id_quad_base = quad.x*16 + quad.y*8;
                parameter_t north_id
                        = memory_id_quad_base
                        + pod_in_quad_base[pod_in_quad.y][pod_in_quad.x];

                parameter_t south_id = north_id+1;
                hb_mc_coordinate_t dram;
                hb_mc_config_pod_foreach_dram(dram, pod, &mc->config)
                {
                        hb_mc_idx_t id = hb_mc_config_dram_id(&mc->config, dram);
                        cache_id_to_memory_id[id] =
                                hb_mc_config_is_dram_north(&mc->config, dram) ?
                                north_id :
                                south_id ;

                        cache_id_to_bank_id[id] = id % caches_per_channel;
                }
        }
        return HB_MC_SUCCESS;
}

/**
 * Initializes a specialized DRAM bank to channel map 1x1 pod model of the BigBlade Chip
 */
static
int hb_mc_dma_init_pod_X1Y1_X16_hbm(hb_mc_manycore_t *mc)
{
        const hb_mc_config_t *cfg = &mc->config;
        hb_mc_coordinate_t pod = {0,0};
        parameter_t west_id = 0, east_id = 1;

        hb_mc_idx_t bx = hb_mc_config_pod_vcore_origin(cfg, pod).x;
        hb_mc_coordinate_t dram;
        hb_mc_config_pod_foreach_dram(dram, pod, cfg)
        {
                hb_mc_idx_t id = hb_mc_config_dram_id(cfg, dram);
                hb_mc_idx_t pid = hb_mc_config_pod_dram_id(cfg, dram);

                int east_not_west = (dram.x - bx) >= cfg->pod_shape.x/2;
                cache_id_to_memory_id[id] =
                        east_not_west ?
                        east_id :
                        west_id ;

                cache_id_to_bank_id[id] =
                        east_not_west
                        ? (hb_mc_config_is_dram_north(cfg, dram)
                           ? (dram.x-bx) - (cfg->pod_shape.x/2)
                           : (dram.x-bx))
                        : (hb_mc_config_is_dram_north(cfg, dram)
                           ? (dram.x-bx)
                           : (dram.x-bx) + (cfg->pod_shape.x/2));
        }

        return HB_MC_SUCCESS;
}

/**
 * Initializes a specialized DRAM bank to channel map for the BigBlade Chip with wormhole test memory
 */
static
int hb_mc_dma_init_pod_X4Y4_X16_test_mem(hb_mc_manycore_t *mc)
{
        hb_mc_coordinate_t pod;
        const hb_mc_config_t *cfg = &mc->config;

        unsigned long test_memories = hb_mc_config_get_dram_channels(cfg);
        unsigned long caches_per_test_mem =
                hb_mc_vcache_num_caches(mc) / test_memories;

        unsigned long test_mems_per_row =
                2 * (cfg->pods.x/2); // split by east-west, counting north and south

        hb_mc_config_foreach_pod(pod, cfg)
        {
                int east_not_west = pod.x >= cfg->pods.x/2;
                int bx = hb_mc_config_get_origin_vcore(cfg).x + east_not_west * (cfg->pod_shape.x * cfg->pods.x/2);
                hb_mc_coordinate_t vcache;
                hb_mc_config_pod_foreach_dram(vcache, pod, cfg)
                {
                        int south_not_north = hb_mc_config_is_dram_south(cfg, vcache);
                        // mod with the ruche factor
                        int ruche_id = vcache.x & 1;
                        int bank = (vcache.x-bx) >> 1;
                        int memory = east_not_west ? test_memories/2 : 0;
                        memory += pod.y * test_mems_per_row;
                        memory += (test_mems_per_row/2) * south_not_north;
                        memory += ruche_id;

                        unsigned long vcache_id = hb_mc_config_dram_id(cfg, vcache);
                        cache_id_to_memory_id[vcache_id] = memory;
                        cache_id_to_bank_id[vcache_id] = bank;
                        dma_pr_dbg(mc, "%s: mapping vcache @ (%d,%d) in pod (%d,%d) to memory %d and bank %d\n",
                                   __func__, vcache.x, vcache.y, pod.x, pod.y, memory, bank);
                }
        }

        return HB_MC_SUCCESS;
}

/**
 * A default DRAM bank to channel map setup - works for most configurations we use
 */
static
int hb_mc_dma_init_default(hb_mc_manycore_t *mc)
{
        for (unsigned long cache_id = 0; cache_id <  hb_mc_vcache_num_caches(mc); cache_id++)
        {
                unsigned long caches_per_channel =
                        hb_mc_vcache_num_caches(mc) /
                        hb_mc_config_get_dram_channels(&mc->config);

                cache_id_to_memory_id[cache_id] = cache_id / caches_per_channel;
                cache_id_to_bank_id[cache_id] = cache_id % caches_per_channel;
        }

        return HB_MC_SUCCESS;
}

int hb_mc_dma_init(hb_mc_manycore_t *mc)
{
        cache_id_to_memory_id = new parameter_t [hb_mc_vcache_num_caches(mc)];
        cache_id_to_bank_id   = new parameter_t [hb_mc_vcache_num_caches(mc)];

        if (mc->config.memsys.id == HB_MC_MEMSYS_ID_HBM2
            && mc->config.pod_shape.x == 16
            && mc->config.pod_shape.y == 8) {
                if (mc->config.pods.x == 4 && mc->config.pods.y == 4) {
                        return hb_mc_dma_init_pod_X4Y4_X16_hbm(mc);
                } else if (mc->config.pods.x == 1 && mc->config.pods.y == 1) {
                        return hb_mc_dma_init_pod_X1Y1_X16_hbm(mc);
                } else {
                        // Pod shape not recognized
                        return HB_MC_FAIL;
                }
        } else if (mc->config.memsys.id == HB_MC_MEMSYS_ID_TESTMEM
                   && mc->config.pod_shape.x == 16
                   && mc->config.pod_shape.y == 8) {
                // 4x4 case
                if (mc->config.pods.x == 4
                    && mc->config.pods.y == 4) {
                        return hb_mc_dma_init_pod_X4Y4_X16_test_mem(mc);
                } else {
                        // for now, we don't support this
                        mc->config.memsys.feature_dma = 0;
                        return HB_MC_SUCCESS;
                }
        } else {
                return hb_mc_dma_init_default(mc);
        }
}

/**
 * Given an NPA that maps to DRAM, return a buffer that holds the data for that address.
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  npa    A valid hb_mc_npa_t - must be an L2 cache coordinate
 * @param[in]  sz     The number of bytes to write to manycore hardware - used for sanity check
 * @param[out] buffer The valid buffer
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
static int hb_mc_dma_npa_to_buffer(hb_mc_manycore_t *mc, const hb_mc_npa_t *npa, size_t sz,
                                        unsigned char **buffer)
{
        /*
          Our system supports having multiple caches per memory channel.
          Currently, we do this by splitting the channels evenly into even 'banks' for each cache.

          IF THE ADDRESS MAPPING SCHEME FROM CACHES TO DRAM CHANGES THIS FUNCTION WILL BREAK!!!!!

          As of the time of this writing we are in the process of designing the memory system.
          So take note...
        */

        /*
          Get system parameters for performing the address mapping
        */
        const hb_mc_config_t *cfg = hb_mc_manycore_get_config(mc);
        unsigned long caches = hb_mc_vcache_num_caches(mc);
        unsigned long channels = hb_mc_config_get_dram_channels(cfg);
        unsigned long caches_per_channel = caches/channels;

        dma_pr_dbg(mc, "%s: caches = %lu, channels = %lu, caches_per_channel = %lu\n",
                        __func__, caches, channels, caches_per_channel);

        /*
          Figure out which memory channel and bank this NPA maps to.
        */
        hb_mc_idx_t cache_id = hb_mc_config_dram_id(cfg, hb_mc_npa_get_xy(npa)); // which cache
        parameter_t id = cache_id_to_memory_id[cache_id];
        parameter_t bank = cache_id_to_bank_id[cache_id]; // which bank within channel

        /*
          Use the backdoor to our non-synthesizable memory.
        */
        Memory *memory = bsg_mem_dma_get_memory(id);
        parameter_t bank_size = memory->size()/caches_per_channel;

        hb_mc_epa_t epa = hb_mc_npa_get_epa(npa);
        char npa_str[256];

        if (memory == nullptr) {
                dma_pr_err(mc, " %s: Could not get the memory for endpoint at %s\n",
                                __func__, hb_mc_npa_to_string(npa, npa_str, sizeof(npa_str)));

                return HB_MC_FAIL;
        }

        // this is the address that comes out of cache_to_test_dram_tx
        address_t cache_addr = bank*bank_size + epa;
        address_t addr = hb_mc_memsys_map_to_physical_channel_address(&cfg->memsys, cache_addr);


        dma_pr_dbg(mc, "%s: Mapped %s to Channel %2lu, Address 0x%08lx\n",
                        __func__, hb_mc_npa_to_string(npa, npa_str, sizeof(npa_str)), id, addr);

        /*
          Don't overflow memory if you can help it.
        */
        assert(addr + sz <= memory->size());
        *buffer = memory->get_ptr(addr);

        return HB_MC_SUCCESS;
}

/**
 * Write memory out to manycore DRAM via C++ backdoor
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  npa    A valid hb_mc_npa_t - must be an L2 cache coordinate
 * @param[in]  data   A buffer to be written out manycore hardware
 * @param[in]  sz     The number of bytes to write to manycore hardware
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_dma_write(hb_mc_manycore_t *mc,
                    const hb_mc_npa_t *npa,
                    const void *data, size_t sz)
{
        unsigned char *membuffer;
        int err = hb_mc_dma_npa_to_buffer(mc, npa, sz, &membuffer);
        if (err != HB_MC_SUCCESS)
                return err;

        char npa_str[256];

        dma_pr_dbg(mc, "%s: Writing %3zu bytes to %s\n",
                        __func__, sz, hb_mc_npa_to_string(npa, npa_str, sizeof(npa_str)));

        memcpy(reinterpret_cast<void*>(membuffer), data, sz);

        return HB_MC_SUCCESS;
}


/**
 * Read memory from manycore DRAM via C++ backdoor
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  npa    A valid hb_mc_npa_t - must be an L2 cache coordinate
 * @param[in]  data   A host buffer to be read into from manycore hardware
 * @param[in]  sz     The number of bytes to read from manycore hardware
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_dma_read(hb_mc_manycore_t *mc, 
                   const hb_mc_npa_t *npa,
                   void *data, size_t sz)
{
        unsigned char *membuffer;
        int err = hb_mc_dma_npa_to_buffer(mc, npa, sz, &membuffer);
        if (err != HB_MC_SUCCESS)
                return err;

        char npa_str[256];

        dma_pr_dbg(mc, "%s: Reading %3zu bytes from %s\n",
                        __func__, sz, hb_mc_npa_to_string(npa, npa_str, sizeof(npa_str)));

        memcpy(data, reinterpret_cast<void*>(membuffer), sz);

        return HB_MC_SUCCESS;
}

