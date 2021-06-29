#pragma once
#include <set>
#include <new>
#include "HammerBlade.hpp"
#include "WGraph.hpp"
#include "bfs/sparse_set.h"


namespace BFS {
    class BFSSparseSet {
    public:
        BFSSparseSet(const std::set<int> &&set,
                     int cap):
            _set(set),
            _hb(hammerblade::host::HammerBlade::Get()) {
            init_kset(cap);
            
        }

        BFSSparseSet(const std::set<int> &set,
                     int cap) :
            _set(set),
            _hb(hammerblade::host::HammerBlade::Get()) {
            init_kset(cap);
        }

        // destructor
        ~BFSSparseSet(){
            free(_kset);
        }

        BFSSparseSet(const BFSSparseSet &other) = delete;
        BFSSparseSet& operator=(const BFSSparseSet &other) = delete;        
        BFSSparseSet(BFSSparseSet &&other) {
            _set = std::move(other._set);

            _hb = other._hb;
            other._hb = nullptr;

            _kset = other._kset;
            other._kset = NULL;

            _set_dev = other._set_dev;
            other._set_dev = 0;            
        } 

        BFSSparseSet& operator=(BFSSparseSet &&other) {
            _set = std::move(other._set);

            _hb = other._hb;
            other._hb = nullptr;

            _kset = other._kset;
            other._kset = NULL;

            _set_dev = other._set_dev;
            other._set_dev = 0;
            return *this;
        }
                
        const std::set<int> &set() const { return _set; }
        kernel_sparse_set_ptr_t dev() const { return _set_dev; }
        
        // format the set on the device
        void formatOnDevice();
        
    private:
        void init_kset(int cap) {
            _kset = reinterpret_cast<sparse_set_t*>(malloc(sizeof(*_kset) + cap*sizeof(int)));
            if (_kset == NULL) {
                throw std::bad_alloc();
            }
            _kset->set_size = _set.size();
            _kset->set_cap = cap;

            int i = 0;
            for (int m : _set) {
                _kset->members[i] = m;
                i++;
            }
        }

        std::set<int>                       _set;
        sparse_set_t                       *_kset;
        kernel_sparse_set_ptr_t             _set_dev;
        hammerblade::host::HammerBlade::Ptr _hb;
    };
}
