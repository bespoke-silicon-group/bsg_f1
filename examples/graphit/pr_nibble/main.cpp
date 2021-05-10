#include "pr.hpp"

//#define DEBUG

#define VERIFY 0

#ifdef DEBUG
#define X 1 
#define Y 1
#else
#define X 16 //tile group dim X 
#define Y 8 // tile group dim Y
#endif

#define ROOT 6 
#define NUM_LOCKS 1024 //width of manycore * 64

GraphHB edges; 
GlobalScalar<hb_mc_eva_t> p_dev;
GlobalScalar<hb_mc_eva_t> old_rank_dev;
GlobalScalar<hb_mc_eva_t> new_rank_dev;
GlobalScalar<hb_mc_eva_t> out_degree_dev;

#include "pr_host.hpp"

int test_pr_nibble(int argc, char ** argv){
    InputParser input(argc, argv);
    if(!input.cmdOptionExists("-g")){
        std::cerr << "no input args\n";
        return 0;
    }
    std::string ucode_path = input.getRISCVFile();

    int iter = 0;
    std::string iterstrbase = "iteration-";
    auto pos = ucode_path.find(iterstrbase);
    auto iterstr = ucode_path.substr(pos+iterstrbase.size(), std::string::npos);
    std::stringstream ss(iterstr);
    ss >> iter;
    std::cerr << "iteration: " << iter << std::endl;

    int version = 0; //default to vertex pull
    if(ucode_path.find("push") != std::string::npos) {
        version = 1;
    }
    int hybrid = 0; //default to vertex pull
    if(ucode_path.find("hybrid") != std::string::npos) {
        hybrid = 1;
    }
    std::cerr << "version: " << version << std::endl;
    std::cerr << "hybrid: " << hybrid << std::endl;
    std::cerr << "load microcode" << std::endl;
    hammerblade::builtin_loadMicroCodeFromFile(ucode_path);

    std::cerr << "load graph" << std::endl;
    std::string graph_f = input.getCmdOption("-g");
    edges = hammerblade::builtin_loadEdgesFromFileToHB (graph_f.c_str()); 

    std::cerr << "size of graph: " << std::endl;
    std::cerr << edges.num_nodes() << std::endl;
    std::cerr << edges.num_edges() << std::endl; 

    std::cerr << "init global scalars" << std::endl; 
    p_dev = GlobalScalar<hb_mc_eva_t>("p");
    hammerblade::init_global_array<float>(hammerblade::builtin_getVerticesHB(edges), p_dev);
    old_rank_dev = GlobalScalar<hb_mc_eva_t>("old_rank");
    hammerblade::init_global_array<float>(hammerblade::builtin_getVerticesHB(edges), old_rank_dev);
    new_rank_dev = GlobalScalar<hb_mc_eva_t>("new_rank");
    hammerblade::init_global_array<float>(hammerblade::builtin_getVerticesHB(edges), new_rank_dev);
    out_degree_dev = GlobalScalar<hb_mc_eva_t>("out_degree");
    hammerblade::init_global_array<int32_t>(hammerblade::builtin_getVerticesHB(edges), out_degree_dev);
    
    std::cerr << "init locks" << std::endl;
    GlobalScalar<hb_mc_eva_t> glbl_locks = GlobalScalar<hb_mc_eva_t>("locks");
    hammerblade::init_global_array<std::atomic<int>>(NUM_LOCKS, glbl_locks);
    std::atomic<int> tmp_a[NUM_LOCKS] = {};
    Device::Ptr device = Device::GetInstance();
    int start_vertex = ROOT;
    Vector<int32_t> frontier = Vector<int32_t>(hammerblade::builtin_getVerticesHB(edges));

    std::vector<int32_t> hfrontier(edges.num_nodes(), 0);
    std::vector<float> p(edges.num_nodes(), (float) 0.0);
    std::vector<float> new_rank(edges.num_nodes(), (float) 0.0);
    std::vector<float> old_rank(edges.num_nodes(), (float) 0.0);
    std::vector<int32_t> out_degs = edges.get_out_degrees();

    //compute up to current iter on host
    hfrontier[start_vertex] = 1;
    new_rank[start_vertex] = (float) 1.0;
    old_rank[start_vertex] = (float) 1.0;
    host_pr_calc(p, old_rank, new_rank, hfrontier, iter);

    //copy all variables at their current state to device
    frontier.copyToDevice(hfrontier.data(), hfrontier.size());
    hammerblade::write_global_buffer_dma<float>(p.data(), p_dev, p.size());  
    hammerblade::write_global_buffer_dma<float>(old_rank.data(), old_rank_dev, old_rank.size());    
    hammerblade::write_global_buffer_dma<float>(new_rank.data(), new_rank_dev, new_rank.size());    
    hammerblade::write_global_buffer_dma<int32_t>(out_degs.data(), out_degree_dev, out_degs.size());    
    //initialize locks for atomics on device
    hammerblade::write_global_buffer_dma<std::atomic<int>>(tmp_a, glbl_locks, NUM_LOCKS);

    device->freeze_cores();
    device->write_dma();
    device->unfreeze_cores();
    //determine push or pull traversal for this iteration 
    if(hybrid) { 
        int num_items = std::count(hfrontier.begin(), hfrontier.end(), 1);
        int dir = calculate_direction(num_items, hfrontier, edges, edges.num_nodes(), edges.num_edges());
        if(dir){ 
            version = 0; //pull
        } else {
            version = 1; //push
        }
    }

    std::cerr << "start of while loop\n";
    int tag_c = 0;
    int f_sz = 0;
    switch(version) {
        case 0: //vertex pull
            std::cerr << "pull kernel\n";
            std::cerr << "preloading the cache\n";
            device->enqueueJob("prefetch", hb_mc_dimension(X,Y),{edges.getInIndicesAddr() , edges.getInNeighborsAddr(), frontier.getAddr(), edges.num_nodes(), edges.num_edges()});
            device->runJobs();
            std::cerr << "run update self vertex kernel\n";
            device->enqueueJob("updateself_kernel",hb_mc_dimension(X,Y), {frontier.getAddr(), edges.num_nodes(), tag_c});
            device->runJobs();
            tag_c++;
            std::cerr << "run update edges kernel on iter : " << iter << "\n";
            device->enqueueJob("edgeset_apply_pull_parallel_from_vertexset_call", hb_mc_dimension(X,Y),{edges.getInIndicesAddr() , edges.getInNeighborsAddr(), frontier.getAddr(), edges.num_nodes(), edges.num_edges(), edges.num_nodes(), tag_c});
            device->runJobs();
            tag_c++;
            std::cerr << "create next frontier\n";
            device->enqueueJob("filter_frontier_where_call", hb_mc_dimension(X,Y),{frontier.getAddr(), edges.num_nodes(), edges.num_edges(), tag_c});
            device->runJobs();
            std::cerr << "swap arrays\n";
            hammerblade::swap_global_arrays<float>(new_rank_dev, old_rank_dev);
            f_sz = builtin_getVertexSetSizeHB(frontier, edges.num_nodes());
            std::cerr << "size of frontier after iteration " << iter << " : " << f_sz << std::endl;
            break;
        case 1: //vertex push
            std::cerr << "push kernel\n";
            std::cerr << "preloading the cache\n";
            device->enqueueJob("prefetch", hb_mc_dimension(X,Y),{edges.getOutIndicesAddr() , edges.getOutNeighborsAddr(), frontier.getAddr(), edges.num_nodes(), edges.num_edges()});
            device->runJobs();
            std::cerr << "run update self vertex kernel\n";
            device->enqueueJob("updateself_kernel",hb_mc_dimension(X,Y), {frontier.getAddr(), edges.num_nodes(), tag_c});
            device->runJobs();
            tag_c++;
            std::cerr << "run update edges kernel on iter : " << iter << "\n";
            device->enqueueJob("edgeset_apply_push_parallel_from_vertexset_call", hb_mc_dimension(X,Y),{edges.getOutIndicesAddr() , edges.getOutNeighborsAddr(), frontier.getAddr(), edges.num_nodes(), edges.num_edges(), edges.num_nodes(), tag_c}); 
            device->runJobs();
            tag_c++;
            std::cerr << "create next frontier\n";
            device->enqueueJob("filter_frontier_where_call", hb_mc_dimension(X,Y),{frontier.getAddr(), edges.num_nodes(), edges.num_edges(), tag_c});
            device->runJobs();
            std::cerr << "swap arrays\n";
            hammerblade::swap_global_arrays<float>(new_rank_dev, old_rank_dev);
            f_sz = builtin_getVertexSetSizeHB(frontier, edges.num_nodes());
            std::cerr << "size of frontier after iteration " << iter << " : " << f_sz << std::endl;
            break;
    }
    if(VERIFY) {
        ofstream ver_file;
        ver_file.open("./rank.txt");
        float host_rank[edges.num_nodes()];
        hammerblade::read_global_buffer_dma<float>(host_rank, old_rank_dev, edges.num_nodes());
        for(int i = 0; i < edges.num_nodes(); i++) {
            ver_file << host_rank[i] << std::endl;
        }
        ver_file.close();  
    }
    device->finish(); 
    return 0;
}

declare_program_main("test_pr_nibble", test_pr_nibble); 
