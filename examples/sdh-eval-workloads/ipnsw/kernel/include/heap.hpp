#pragma once
#include <array>
#include <algorithm>

template <typename T, typename Comparitor>
class DynHeap {
public:
    DynHeap(T *data, int N):
        _data(data),
        _data_N(N),
        _n(0){
    }

    void push(T i) {
        _data[_n++] = i;
        std::push_heap(_data, _data+_n, Comparitor());
        if (_n > _data_N) pop();
    }

    T pop() {
        std::pop_heap(_data, _data+_n--, Comparitor());
        return _data[_n];
    }

    T top() const {
        return _data[0];
    }

    bool empty() const {
        return _n == 0;
    }

    int size() const {
        return _n;
    }

    int _n;
    int _data_N;
    T  *_data;
};
