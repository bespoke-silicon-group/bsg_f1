#pragma once
#include <algorithm>
#include <atomic>
template<typename T, typename Comparitor>
class DynSet {
public:
    DynSet(T *data, int N):
        _data(data),
        _data_N(N),
        _n(0) {
    }

    void insert(T i) {
        _data[_n++] = i;
        std::sort(_data, _data+_n, Comparitor());
    }

    bool in(T i) {
        return std::binary_search(_data, _data+_n, i, Comparitor());
    }

    int size() const {
        return _n;
    }

    T    *_data;
    int   _n;
    int   _data_N;
};

template<typename T>
class DenseSet {
public:
    DenseSet(int *data):
        _data(data) {
    }

    void insert(T i) {
        _data[i] = 1;
    }

    void atomic_insert(T i) {
        insert(i);
    }

    bool in(T i) {
        return _data[i] == 1;
    }

    int *_data;
};

template<typename T>
class DenseSet_v1 {
public:
    DenseSet_v1(int *data) :
        _data(data){
    }

    void insert(T i) {
        _data[word(i)] |= (1 << bit(i));
    }

    void atomic_insert(T i) {
        int *ptr = &_data[word(i)];
        int r    = 1 << bit(i);
        asm volatile ("amoor.w x0, %[r], %[ptr]" :
                      :
                      [r] "r" (r),
                      [ptr] "m" (*ptr));
        return;
    }

    bool in(T i) {
        return _data[word(i)] & (1 << bit(i));
    }

    int word(T i) const {
        return  i >> 5;
    }

    int bit(T i) const {
        return i & 31;
    }
    int *_data;
};
