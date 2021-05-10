//function to compute pr-nibble on host up to current iter
#pragma once
#include <iostream>
#include <fstream>

inline void host_pr_calc(std::vector<float> & p, std::vector<float> & old_rank, std::vector<float> & new_rank, std::vector<int> & frontier, int iter) {
        float alpha = (float) 0.15;
        float epsilon = (float) 1e-06;
        auto g = edges.getHostGraph();
        int * in_neigh = g.in_neighbors_shared_.get();
        int ** in_index = g.in_index_shared_.get();
        for(int i = 0; i < iter; i++) {
                new_rank.assign(old_rank.begin(), old_rank.end());
                //print out iteration and size:
                int num_items = std::count(frontier.begin(), frontier.end(), 1);
                std::cerr << "on iteration: " << i << " with frontier size: " << num_items << std::endl;
                //update_self
                for(int v = 0; v < g.num_nodes(); v++) {
                        if(frontier[v]) {
                            p[v] += (2.0 * alpha) / (1.0    + alpha) * old_rank[v];
                            new_rank[v] = (float) 0.0 ;
                        }
                }
                //update edges
                for(int d = 0; d < g.num_nodes(); d++) {
                        for(int s : g.in_neigh(d)) {
                                if(frontier[s]){
                                        float update = ((1.0 - alpha) / (1.0    + alpha)) * old_rank[s];
                                        update = update / ((float) g.out_degree(s));
                                        new_rank[d] += update;
                                }
                        }
                }
                old_rank.assign(new_rank.begin(), new_rank.end());
                //update frontier
                for(int v = 0; v < g.num_nodes(); v++) {
                        frontier[v] = 0;
                        if(g.out_degree(v) > 0 && old_rank[v] >= (((float) g.out_degree(v)) * epsilon)) {
                                frontier[v] = 1;
                        }
                }
        }
        int num_items = std::count(frontier.begin(), frontier.end(), 1);
        std::cerr << "returning with frontier size: " << num_items << std::endl;
}
