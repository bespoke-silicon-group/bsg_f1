#include "bfs/BFSSparseSet.hpp"
namespace BFS {
    void BFSSparseSet::formatOnDevice() {
        _set_dev = _hb->alloc(sizeof(*_kset) + _kset->set_cap*sizeof(int));
        _hb->push_write(_set_dev, _kset, sizeof(*_kset) + _kset->set_cap*sizeof(int));        
    }
}
