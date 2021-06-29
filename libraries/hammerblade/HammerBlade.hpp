#pragma once
#include <RuntimeError.hpp>
#include <bsg_manycore_cuda.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_tile.h>
#include <bsg_manycore_coordinate.h>
#include <memory>
#include <vector>
#include <cstdlib>
#include <cstring>

namespace hammerblade {
    namespace host {
        class Dim {
        private:
            hb_mc_dimension_t _dim;

        public:
            explicit Dim(hb_mc_idx_t x, hb_mc_idx_t y) {
                _dim = HB_MC_COORDINATE(x,y);
            }

            explicit Dim(hb_mc_dimension_t d) {
                _dim = d;
            }

            Dim operator/(Dim l) {
                Dim r(x()/l.x(),y()/l.y());
                return r;
            }

            hb_mc_idx_t x() const { return hb_mc_dimension_get_x(_dim); }
            hb_mc_idx_t y() const { return hb_mc_dimension_get_y(_dim); }
            hb_mc_dimension_t hb_mc_dim() const { return _dim; }
        };

        class HammerBlade {
        public:
            HammerBlade() :
                _application_data(NULL),
                _application_size(0),
                _has_app_loaded(false) {

                _cuda = new hb_mc_device_t;
                int err = hb_mc_device_init(_cuda, "HammerBlade", 0);
                if (err != HB_MC_SUCCESS)
                    throw RuntimeError(err);

                hb_mc_device_set_default_pod(_cuda, 0);
            }

            ~HammerBlade() {
                int err = hb_mc_device_finish(_cuda);
                if (err != HB_MC_SUCCESS)
                    throw RuntimeError(err);
                delete _cuda;

                if (_application_data != NULL) {
                    std::free(_application_data);
                    _application_data = NULL;
                    _application_size = 0;
                }
            }
            /////////////////////////////
            // Configuration Interface //
            /////////////////////////////
            int num_vanilla_cores() const {
                return vanilla_cores_dimension().x
                    *  vanilla_cores_dimension().y;
            }

            ///////////////////////////
            // Application Interface //
            ///////////////////////////
            bool has_application_loaded() const {
                return _has_app_loaded;
            }

            void load_application(const char *binpath) {
                load_application(std::string(binpath));
            }

            void load_application(const std::string & binpath) {
                int err;
                err = hb_mc_loader_read_program_file(binpath.c_str(),
                                                     &_application_data,
                                                     &_application_size);
                if (err != HB_MC_SUCCESS)
                    throw RuntimeError(err);

                err = hb_mc_device_program_init_binary(_cuda,
                                                       binpath.c_str(),
                                                       _application_data,
                                                       _application_size,
                                                       "HammerBlade Allocator",
                                                       0);
                if (err != HB_MC_SUCCESS)
                    throw RuntimeError(err);

                _has_app_loaded = true;
            }

            ///////////////////////
            // Memory Allocation //
            ///////////////////////
            hb_mc_eva_t alloc(size_t size) {
                hb_mc_eva_t mem;
                int err = hb_mc_device_malloc(_cuda, size, &mem);
                if (err != HB_MC_SUCCESS)
                    throw RuntimeError(err);
                return mem;
            }

            void free(hb_mc_eva_t mem) {
                int err = hb_mc_device_free(_cuda, mem);
                if (err != HB_MC_SUCCESS)
                    throw RuntimeError(err);
            }

            ////////////////
            // Read/Write //
            ////////////////

        private:
            class WriteJob {
            public:
                WriteJob(hb_mc_eva_t dst, const void *src, size_t size):
                    _d_addr(dst),
                    _data(size) {
                    memcpy(&_data[0], src, size);
                }

                hb_mc_dma_htod_t to_job() const {
                    hb_mc_dma_htod_t job = {
                        .d_addr = _d_addr,
                        .h_addr = &_data[0],
                        .size = _data.size()
                    };
                    return job;
                }

            private:
                hb_mc_eva_t                _d_addr;
                std::vector<unsigned char> _data;
            };

            std::vector<WriteJob>         _write_jobs;
            std::vector<hb_mc_dma_dtoh_t> _read_jobs;
        public:
            void push_write(hb_mc_eva_t dst, const void *src, size_t size) {
                _write_jobs.push_back(WriteJob(dst, src, size));
            }

            void sync_write() {
                std::vector<hb_mc_dma_htod_t> jobs;
                for (auto &j : _write_jobs)
                    jobs.push_back(j.to_job());

                int err = hb_mc_device_dma_to_device(_cuda, &jobs[0], jobs.size());
                if (err != HB_MC_SUCCESS)
                    throw RuntimeError(err);
            }

            void push_read(hb_mc_eva_t src, void *dst, size_t size) {
                hb_mc_dma_dtoh_t job = { .d_addr = src, .h_addr = dst, .size = size};
                _read_jobs.push_back(job);
            }

            void sync_read() {
                int err = hb_mc_device_dma_to_host(_cuda, &_read_jobs[0], _read_jobs.size());
                if (err != HB_MC_SUCCESS)
                    throw RuntimeError(err);
            }

            void sync_rw() {
                sync_read();
                sync_write();
            }

            void write(hb_mc_eva_t dst, const void *src, size_t size) {
                int err = hb_mc_device_memcpy_to_device(_cuda, dst, src, size);
                if (err != HB_MC_SUCCESS)
                    throw RuntimeError(err);
            }

            void read(hb_mc_eva_t src, void *dst, size_t size) {
                int err = hb_mc_device_memcpy_to_host(_cuda, dst, src, size);
                if (err != HB_MC_SUCCESS)
                    throw RuntimeError(err);
            }

            //////////////////////
            // Kernel Execution //
            //////////////////////
        private:
            void _push_job_unpack(std::vector<hb_mc_eva_t>&v) {
                // do nothing (calling a kernel without arguments)
            }

            template <typename T>
            void _push_job_unpack(std::vector<hb_mc_eva_t>&v, T arg) {
                v.push_back(static_cast<hb_mc_eva_t>(arg));
            }

            template <typename T, typename ... Args>
            void _push_job_unpack(std::vector<hb_mc_eva_t>&v, T arg, Args ... args) {
                _push_job_unpack(v, arg);
                _push_job_unpack(v, args...);
            }

        public:
            template <typename ... Args>
            void push_job(Dim grid, Dim group, const std::string & kname, Args... args) {
                std::vector<hb_mc_eva_t> argv;
                _push_job_unpack(argv, args...);

                push_jobv(grid, group, kname, argv);
            }

            void push_jobv(Dim grid, Dim group, const std::string &kname,
                           const std::vector<hb_mc_eva_t> & argv) {
                int err =
                    hb_mc_kernel_enqueue(_cuda,
                                         grid.hb_mc_dim(),
                                         group.hb_mc_dim(),
                                         kname.c_str(),
                                         argv.size(),
                                         argv.data());

                if (err != HB_MC_SUCCESS)
                    throw RuntimeError(err);
            }

            void exec() {
                int err = hb_mc_device_tile_groups_execute(_cuda);
                if (err != HB_MC_SUCCESS)
                    throw RuntimeError(err);
            }

            /////////////////////////////
            // Configuration Interface //
            /////////////////////////////
        public:
            hb_mc_eva_t cache_block_size() const {
                return hb_mc_config_get_vcache_block_size(config());
            }

            /////////////////////////////////
            // Special Functions Interface //
            /////////////////////////////////
        private:
            template<typename FunctionType>
            void foreach_tile(FunctionType func) {
                const hb_mc_mesh_t *m = mesh();
                for (hb_mc_idx_t x = 0; x < m->dim.x; x++)
                    for (hb_mc_idx_t y = 0; y < m->dim.y; y++)
                        func(hb_mc_coordinate(m->origin.x+x,m->origin.y+y));
            }

        public:
            void freeze_tiles() {
                foreach_tile([this](hb_mc_coordinate_t tile){
                        int err = hb_mc_tile_freeze(_cuda->mc, &tile);
                        if (err != HB_MC_SUCCESS)
                            throw RuntimeError(err);
                    });
            }

            void unfreeze_tiles() {
                foreach_tile([this](hb_mc_coordinate_t tile){
                        int err = hb_mc_tile_unfreeze(_cuda->mc, &tile);
                        if (err != HB_MC_SUCCESS)
                            throw RuntimeError(err);
                    });
            }

            ///////////////////////
            // Lookup Symbol API //
            ///////////////////////
            bool has_kernel(std::string kernel_name) const {
                if (!has_application_loaded()) return false;

                hb_mc_eva_t _;

                int err = hb_mc_loader_symbol_to_eva(_application_data,
                                                     _application_size,
                                                     kernel_name.c_str(),
                                                     &_);

                return err == HB_MC_SUCCESS ? true : false;
            }

            ////////////////////////
            // Singleton Inteface //
            ////////////////////////
        public:
#ifdef COSIM
            using Ptr = HammerBlade *;
            using CPtr = const HammerBlade *;
#else
            using Ptr  = std::shared_ptr<HammerBlade>;
            using CPtr = std::shared_ptr<const HammerBlade>;
#endif
            // singleton implementation with smart pointer
            static
            Ptr Get() {
                static Ptr instance = nullptr;
                if (instance == nullptr)
#ifdef COSIM
                    instance = new HammerBlade;
#else
                    instance = std::make_shared<HammerBlade>();
#endif
                return instance;
            }

            static
            void Test(const char *binpath) {
                {
                    Ptr hb = Get();
                    std::cout << "HammerBlade instance with "
                              << hb->num_vanilla_cores()
                              << " "
                              << "cores"
                              << std::endl;
                    hb->load_application(std::string(binpath));

                    const char w_message[] = "hello, world!";
                    hb_mc_eva_t v = hb->alloc(sizeof(w_message));
                    hb->push_write(v, w_message, sizeof(w_message));
                    hb->sync_rw();

                    char r_message [128];
                    hb->push_read(v, r_message, sizeof(r_message));
                    hb->sync_rw();

                    hb->free(v);
                }
            }

        private:
            hb_mc_dimension_t vanilla_cores_dimension() const {
                return hb_mc_config_get_dimension_vcore(config());
            }

            const hb_mc_config_t * config() const { return hb_mc_manycore_get_config(_cuda->mc); }
            const hb_mc_mesh_t   * mesh()   const {
                auto m = _cuda->pods[_cuda->default_pod_id].mesh;
                if (m == NULL) {
                    throw RuntimeError(HB_MC_UNINITIALIZED);
                }
                return m;
            }

        public:
            Dim physical_dimension() const {
                hb_mc_dimension_t d = vanilla_cores_dimension();
                return Dim(d);
            }

        private:
            // device handle
            hb_mc_device_t *_cuda;

            // application members
            unsigned char * _application_data;
            size_t          _application_size;
            bool _has_app_loaded;
        };
    }
}
