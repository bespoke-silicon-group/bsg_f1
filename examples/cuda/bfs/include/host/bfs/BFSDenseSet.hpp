#pragma once
#include <set>
#include "HammerBlade.hpp"
#include "bfs/types.h"


namespace BFS {
    class BFSDenseSet {
    public:
        BFSDenseSet(const std::set<int> &set,
                    int cap):
            _set(set),
            _kset(cap, 0),
            _hb(hammerblade::host::HammerBlade::Get()) {
            _cap = cap;
            // initialize kset
            for (int m : _set)
                _kset[m] = 1;
        }

        BFSDenseSet(std::set<int> && set,
                    int cap):
            _set(set),
            _kset(cap, 0),
            _hb(hammerblade::host::HammerBlade::Get()){
            _cap = cap;
            // initialize kset
            for (int m : _set)
                _kset[m] = 1;            
        }

        void formatOnDevice() {
            _set_dev = _hb->alloc(_kset.size()*sizeof(int));
            _hb->push_write(_set_dev, &_kset[0],
                            _kset.size()*sizeof(int));
        }

        void updateFromDevice() {
            _hb->push_read(_set_dev, &_kset[0], _kset.size() * sizeof(int));
        }

        std::set<int> setAfterUpdate() const {
            std::set<int> after;
            for (int i = 0; i < _kset.size(); i++)
                if (_kset[i] == 1)
                    after.insert(i);
            return after;
        }

        const std::set<int> & set() const { return _set; }
        
        kernel_int_ptr_t dev() const { return _set_dev; }

    private:
        std::set<int>    _set;
        std::vector<int> _kset;
        kernel_int_ptr_t _set_dev;
        hammerblade::host::HammerBlade::Ptr _hb;
        int              _cap;
    };
}
