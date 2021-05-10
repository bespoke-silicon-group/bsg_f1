#pragma once
#include "IPNSWKernelRunner.hpp"
#include "IPNSWRunner.hpp"
#include "GreedyWalkResults.hpp"

namespace ipnsw {
    class BeamSearchKernelRunner : public IPNSWKernelRunner {
        std::string kernelName(const IPNSWRunner & runner) const {
            return "ipnsw_beam_search";
        }

        Dim tgd(const IPNSWRunner & runner) const {
            return Dim(runner.cfg().grp_x(),
                       runner.cfg().grp_y());
        }

        Dim gd(const IPNSWRunner & runner) const {
            return Dim(runner.cfg().grid_x(),
                       runner.cfg().grid_y());
        }
        std::vector<hb_mc_eva_t> argv(const IPNSWRunner & runner) const {
            int v_curr;
            float d_curr;
            std::vector<int> do_queries = runner._io->do_queries();
            if (do_queries.empty()) {
                v_curr = std::get<GWR_VERT>(GREEDY_WALK_RESULTS[IPNSWRunner::QUERY]);
                d_curr = std::get<GWR_DIST>(GREEDY_WALK_RESULTS[IPNSWRunner::QUERY]);
            } else {
                v_curr = std::get<GWR_VERT>(GREEDY_WALK_RESULTS[do_queries[0]]);
                d_curr = std::get<GWR_DIST>(GREEDY_WALK_RESULTS[do_queries[0]]);
            }

            HammerBlade::Ptr hb = HammerBlade::Get();
            hb->write(runner.v_curr_dev(0), &v_curr, sizeof(v_curr));
            hb->write(runner.d_curr_dev(0), &d_curr, sizeof(d_curr));

            std::vector<hb_mc_eva_t> argv = {
                runner.graph_metadata_dev(),
                runner.db_dev(),
                runner.query_dev(0),
                runner.seen_dev(0),
                runner.v_curr_dev(0),
                runner.d_curr_dev(0),
                runner.candidates_dev(0),
                runner.results_dev(0),
                runner.n_results_dev(0),
            };
            return argv;
        }

    };
}
