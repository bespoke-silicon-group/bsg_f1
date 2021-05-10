#include <bsg_manycore_cuda.h>
namespace ipnsw {
    struct GroupData {
        hb_mc_eva_t seen_mem;
        hb_mc_eva_t candidates_mem;
        hb_mc_eva_t results_mem;
        hb_mc_eva_t curr;
        hb_mc_eva_t n_results;
    };
};
