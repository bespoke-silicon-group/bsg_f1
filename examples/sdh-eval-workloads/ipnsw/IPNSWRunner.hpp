#pragma once
#include "IO.hpp"
#include "HammerBlade.hpp"
#include "IPNSWGraph.hpp"
#include "IPNSWFactory.hpp"
#include "IPNSWKernelRunner.hpp"
#include "IPNSWResultReader.hpp"
#include "GreedyWalkResults.hpp"
#include "GroupData.hpp"
#include <memory>

namespace ipnsw {

    class IPNSWRunnerConfig {
    public:
        typedef enum {
            Dense,
            BitVector,
            Sparse,
        } SetType;

        IPNSWRunnerConfig():
            _set_type(BitVector),
            _grid_x(1),
            _grid_y(1),
            _grp_x(1),
            _grp_y(1) {
        }

        SetType set_type() const { return _set_type; }
        SetType & set_type() { return _set_type; }

        std::string set_type_str() const {
            switch (set_type()) {
            case Dense:
                return "Dense";
            case BitVector:
                return "Dense Bit Vector";
            case Sparse:
                return "Sparse";
            }
        }

        int & grid_x()       { return _grid_x; }
        int   grid_x() const { return _grid_x; }
        int & grid_y()       { return _grid_y; }
        int   grid_y() const { return _grid_y; }

        int & grp_x()       { return _grp_x; }
        int   grp_x() const { return _grp_x; }
        int & grp_y()       { return _grp_y; }
        int   grp_y() const { return _grp_y; }

    private:
        SetType _set_type;
        int     _grid_x;
        int     _grid_y;
        int     _grp_x;
        int     _grp_y;
    };

    class IPNSWRunner {
    public:
        //static constexpr int QUERY = 276; // fewest dot products for greedy walk
        //static constexpr int QUERY = 472; // fewest dot products for beam search
        //static constexpr int QUERY = 427;
        //static constexpr int QUERY = 355;
        //static constexpr int QUERY = 2;
        static constexpr int QUERY = 188;
        //static constexpr int QUERY = 229;
        //static constexpr int QUERY = 490;
        //static constexpr int QUERY = 16;
        //static constexpr int QUERY = 461;
        //static constexpr int QUERY = 470;


        static constexpr size_t CANDIDATES_MAX = 513;
        static constexpr size_t RESULTS_MAX    = 129;

        using HammerBlade = hammerblade::host::HammerBlade;
        using Dim = hammerblade::host::Dim;

        IPNSWRunner(const Parser &p,
                    std::unique_ptr<IPNSWFactory> & fact) :
            IPNSWRunner(p, fact, IPNSWRunnerConfig()) {
        }
        
        IPNSWRunner(const Parser &p,
                    std::unique_ptr<IPNSWFactory> & fact,
                    const IPNSWRunnerConfig &cfg):
            _factory(std::move(fact)),
            _cfg(cfg) {
            _io = std::unique_ptr<IO>(new IO(p));
            _hb = HammerBlade::Get();
            _kernel_runner = _factory->KernelRunner();
            _result_reader = _factory->ResultReader();
        }

        virtual ~IPNSWRunner() { delete _hb; }

        void readInput() {
            auto graphs   = _io->graphs();
            _graphs = {
                Graph(std::move(graphs[3])),
                Graph(std::move(graphs[2])),
                Graph(std::move(graphs[1])),
                Graph(std::move(graphs[0]))
            };

            _db       = _io->database<float,100>();
            _queries  = _io->queries<float,100>();
        }

        void loadProgram() {
            _hb->load_application(ucodePath());
        }

        void initializeDeviceMemoryDB() {
            std::cout << "Initializing database " << std::endl;
            _db_dev = _hb->alloc(_db.size() * sizeof(_db[0]));
            _hb->push_write(_db_dev, &_db[0], _db.size() * sizeof(_db[0]));
        }

        void initializeDeviceMemoryQuery() {
            std::cout << "Initializing query "  << std::endl;

            std::vector<int> do_queries = _io->do_queries();
            if (do_queries.empty()) {
                do_queries = {QUERY};
            }

            _query_dev = _hb->alloc(sizeof(_queries[0]) * do_queries.size());

            for (hb_mc_eva_t qidx = 0; qidx < do_queries.size(); ++qidx) {
                int query = do_queries[qidx];
                _hb->push_write(_query_dev + qidx * sizeof(_queries[query]),
                                &_queries[query],
                                sizeof(_queries[query]));
            }
        }

        size_t seen_dev_size_per_group() const {
            size_t size, words;
            switch (_cfg.set_type()) {                
            case IPNSWRunnerConfig::Dense:
            case IPNSWRunnerConfig::Sparse:                
                return _db.size() * sizeof(int);
            case IPNSWRunnerConfig::BitVector:
                words = _db.size()/32;
                if (_db.size() % 32 != 0)
                    words += 1;
                return words * sizeof(int);
            }
        }
        void initializeDeviceMemorySeen() {
            std::cout << "Initializing seen set " << std::endl;
            for (int i = 0; i < numGroups(); ++i) {
                hb_mc_eva_t dev = _hb->alloc(seen_dev_size_per_group());
                _seen_dev.push_back(dev);
            }
        }

        void initializeDeviceMemoryGraphs() {
            for (auto & graph : _graphs)
                graph.initialize_on_device();

            _graph_metadata_dev = Graph::InitializeMetadataOnDevice(_graphs);
        }

        void initializeDeviceVCurrDCurr() {
            _curr_dev = _hb->alloc(sizeof(GreedyWalkResult) * numGroups());
            hb_mc_eva_t grp = 0;
            std::cout << std::hex;
            std::cout << "_curr_dev=" << std::hex << _curr_dev << std::endl;
            std::cout << "  curr(" << std::dec << grp << ")=" << std::hex <<   curr_dev(grp) << std::endl;
            std::cout << "v_curr(" << std::dec << grp << ")=" << std::hex << v_curr_dev(grp) << std::endl;
            std::cout << "d_curr(" << std::dec << grp << ")=" << std::hex << d_curr_dev(grp) << std::endl;
            std::cout << std::dec;
        }

        size_t candidates_dev_size_per_group() const {
            return sizeof(GreedyWalkResult) * CANDIDATES_MAX;
        }

        void initializeDeviceCandidateDev() {
            for (int i = 0; i < numGroups(); ++i) {
                hb_mc_eva_t dev = _hb->alloc(candidates_dev_size_per_group());
                _candidates_dev.push_back(dev);
            }
        }

        size_t results_dev_size_per_group() const {
            return sizeof(GreedyWalkResult) * RESULTS_MAX;
        }

        void initializeDeviceResultsDev() {
            for (int i = 0; i < numGroups(); ++i) {
                hb_mc_eva_t dev = _hb->alloc(results_dev_size_per_group());
                _results_dev.push_back(dev);
            }
        }

        void initializeDeviceNResultsDev() {
            _n_results_dev = _hb->alloc(sizeof(int) * numGroups());
        }

        void initializeGroupData() {
            _group_data_dev = _hb->alloc(sizeof(GroupData) * numGroups());
            for (int i = 0; i < numGroups(); ++i) {
                GroupData gd = {
                    .seen_mem       = seen_dev(i),
                    .candidates_mem = candidates_dev(i),
                    .results_mem    = results_dev(i),
                    .curr           = curr_dev(i),
                    .n_results      = n_results_dev(i),
                };
                _hb->push_write(group_data_dev(i), &gd, sizeof(gd));
            }
        }

        void initializeDeviceMemory() {
            initializeDeviceMemoryDB();
            initializeDeviceMemoryQuery();
            initializeDeviceMemorySeen();
            initializeDeviceMemoryGraphs();
            initializeDeviceVCurrDCurr();
            initializeDeviceCandidateDev();
            initializeDeviceResultsDev();
            initializeDeviceNResultsDev();
            initializeGroupData();
        }

        void runKernel() {
            _kernel_runner->beforeLaunchKernel(*this);
            // sync
            std::cout << "Starting DMA" << std::endl;
            _hb->sync_rw();
            std::cout << "Launching kernel" << std::endl;
            _kernel_runner->runKernel(*this);
            _kernel_runner->afterLaunchKernel(*this);
        }

        void readResults() {
            _result_reader->readResults(*this);

        }

        void run() {
            readInput();
            loadProgram();
            initializeDeviceMemory();
            runKernel();
            readResults();
        }

        /////////////
        // Getters //
        /////////////
        std::string ucodePath() const {
            return _io->ucode();
        }

        hb_mc_eva_t db_dev() const { return _db_dev; }
        hb_mc_eva_t query_dev(hb_mc_eva_t qidx) const {
            return _query_dev + qidx * sizeof(_queries[qidx]);
        }

        hb_mc_eva_t seen_dev(hb_mc_eva_t grp) const {
            return _seen_dev[grp];
        }

        hb_mc_eva_t curr_dev(hb_mc_eva_t grp = 0) const {
            return _curr_dev + (grp*sizeof(GreedyWalkResult));
        }

        hb_mc_eva_t v_curr_dev(hb_mc_eva_t grp) const {
            return curr_dev(grp) + sizeof(float);
        }
        hb_mc_eva_t d_curr_dev(hb_mc_eva_t grp) const {
            return curr_dev(grp);
        }

        hb_mc_eva_t graph_metadata_dev() const { return _graph_metadata_dev; }

        hb_mc_eva_t candidates_dev(hb_mc_eva_t grp) const {
            return _candidates_dev[grp];
        }

        hb_mc_eva_t results_dev(hb_mc_eva_t grp) const {
            return _results_dev[grp];
        }

        hb_mc_eva_t n_results_dev(hb_mc_eva_t grp) const {
            return _n_results_dev + grp * sizeof(int);
        }
        
        hb_mc_eva_t group_data_dev(hb_mc_eva_t grp) const {
            return _group_data_dev + grp * sizeof(GroupData);
        }

        int numGroups() const { return _kernel_runner->gd(*this).x() * _kernel_runner->gd(*this).y(); }

        const std::vector<std::array<float,100>> & db() const { return _db; }

        const IPNSWRunnerConfig & cfg() const { return _cfg; }
        /////////////
        // Setters //
        /////////////

    private:
        IPNSWRunnerConfig                     _cfg;

    public:
        std::unique_ptr<IO>                   _io;

    private:
        std::vector<Graph>                    _graphs;
        std::vector<std::array<float, 100>>   _db;
        std::vector<std::array<float, 100>>   _queries;
        std::vector<GroupData>                _group_data;
        HammerBlade::Ptr                      _hb;

        // device pointers
        hb_mc_eva_t _db_dev;
        hb_mc_eva_t _query_dev;
        std::vector<hb_mc_eva_t> _seen_dev;
        hb_mc_eva_t _curr_dev;
        hb_mc_eva_t _graph_metadata_dev;
        std::vector<hb_mc_eva_t> _candidates_dev;
        std::vector<hb_mc_eva_t> _results_dev;
        hb_mc_eva_t _n_results_dev;
        hb_mc_eva_t _group_data_dev;

        // composites
        std::unique_ptr<IPNSWKernelRunner> _kernel_runner;
        std::unique_ptr<IPNSWResultReader> _result_reader;
        std::unique_ptr<IPNSWFactory>      _factory;
    };
}
