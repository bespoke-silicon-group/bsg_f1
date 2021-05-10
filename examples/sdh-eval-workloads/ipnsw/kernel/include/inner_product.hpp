#pragma once
#include "bsg_striped_array.hpp"
#include <cmath>
#include <numeric>
#include <bsg_manycore.hpp>
#include "sleep_until_valid.hpp"

template<std::size_t TG_X, std::size_t TG_Y, typename FLOAT_T=float, std::size_t VSIZE=100, std::size_t BSIZE=10>
__attribute__((noinline))
FLOAT_T inner_product(const FLOAT_T *__restrict a, const FLOAT_T *__restrict b)
{
    FLOAT_T r = 0.0;
    for (int i = __bsg_id * BSIZE; i < VSIZE; i += BSIZE * TG_X * TG_Y) {
        #pragma GCC unroll 32
        for (int j = 0; j < BSIZE; ++j) {
            r += a[i + j]*b[i + j];
        }
    }
    return r;
}


template<std::size_t TG_X, std::size_t TG_Y, typename FLOAT_T=float, std::size_t VSIZE=100, std::size_t BSIZE=10>
__attribute__((noinline))
FLOAT_T inner_product_v1(const FLOAT_T *__restrict a,
                         bsg_attr_remote const FLOAT_T *__restrict b)
{
    FLOAT_T r = 0.0;
    for (int i = __bsg_id * BSIZE; i < VSIZE; i += BSIZE * TG_X * TG_Y) {
        #pragma GCC unroll 32
        for (int j = 0; j < BSIZE; ++j) {
            r += a[i + j]*b[i + j];
        }
    }
    return r;
}


template<std::size_t TG_X, std::size_t TG_Y, typename FLOAT_T=float, std::size_t VSIZE=100, std::size_t BSIZE=10>
__attribute__((noinline))
FLOAT_T inner_product_v2(const FLOAT_T *__restrict a,
                         bsg_attr_remote const FLOAT_T *__restrict b)
{
    FLOAT_T r = 0.0;
    for (int i = __bsg_id * BSIZE; i < VSIZE; i += BSIZE * TG_X * TG_Y) {
        #pragma GCC unroll 32
        for (int j = 0; j < BSIZE; ++j) {
            r = fmaf(a[i+j], b[i+j], r);
        }
    }
    return r;
}



template<std::size_t TG_X, std::size_t TG_Y, typename FLOAT_T=float, std::size_t VSIZE=100, std::size_t BSIZE=10>
__attribute__((noinline))
FLOAT_T inner_product_v3(const FLOAT_T *__restrict a,
                         bsg_attr_remote const FLOAT_T *__restrict b)
{
    FLOAT_T r0 = 0.0, r1 = 0.0;
    for (int i = __bsg_id * BSIZE; i < VSIZE; i += 2 * BSIZE * TG_X * TG_Y) {
#pragma bsg_unroll(32)
        for (int j = 0; j < BSIZE; ++j) {
            r0 = fmaf(a[i+j+0*BSIZE], b[i+j+0*BSIZE], r0);
            r1 = fmaf(a[i+j+1*BSIZE], b[i+j+1*BSIZE], r1);
        }
    }
    return r0+r1;
}

template<std::size_t TG_X, std::size_t TG_Y, typename FLOAT_T=float, std::size_t VSIZE=100, std::size_t BSIZE=5, int UNROLL=5>
__attribute__((noinline))
FLOAT_T inner_product_v4(const FLOAT_T *__restrict a,
                         bsg_attr_remote const FLOAT_T *__restrict b)
{
    register FLOAT_T r[UNROLL] = {0};
    for (int i = __bsg_id * BSIZE * UNROLL; i < VSIZE; i += UNROLL * BSIZE * TG_X * TG_Y) {
#pragma bsg_unroll(32)
        for (int j = 0; j < BSIZE; ++j) {
#pragma bsg_unroll(32)
            for (int k =0 ; k < UNROLL; ++k) {
                r[k] = fmaf(a[i+j+k*BSIZE], b[i+j+k*BSIZE], r[k]);
            }
        }
    }
    FLOAT_T rs = 0.0;
    for (int i = 0; i < UNROLL; ++i)
        rs += r[i];
    return rs;
}

template<std::size_t TG_X, std::size_t TG_Y, typename FLOAT_T=float, std::size_t VSIZE=100, std::size_t BSIZE=5, int UNROLL=5>
__attribute__((noinline))
FLOAT_T inner_product_parallel_v1(const FLOAT_T *__restrict a,
                                  bsg_attr_remote const FLOAT_T *__restrict b)
{
    register FLOAT_T r[UNROLL] = {0.0};

    for (int i = __bsg_id * BSIZE * UNROLL; i < VSIZE; i += UNROLL * BSIZE * TG_X * TG_Y) {
#pragma bsg_unroll(32)
        for (int j = 0; j < BSIZE; ++j) {
#pragma bsg_unroll(32)
            for (int k =0 ; k < UNROLL; ++k) {
                r[k] = fmaf(a[i+j+k*BSIZE], b[i+j+k*BSIZE], r[k]);
            }
        }
    }
    FLOAT_T rs = 0.0;
    for (int i = 0; i < UNROLL; ++i)
        rs += r[i];

    return rs;
}


template<typename FLOAT_T=float, std::size_t VSIZE=100, std::size_t BSIZE=5, int UNROLL=5>
__attribute__((noinline))
FLOAT_T inner_product_v4_serial(const FLOAT_T *__restrict a,
                         bsg_attr_remote const FLOAT_T *__restrict b)
{
    register FLOAT_T r[UNROLL] = {0};
    for (int i = 0; i < VSIZE; i += UNROLL * BSIZE) {
#pragma bsg_unroll(32)
        for (int j = 0; j < BSIZE; ++j) {
#pragma bsg_unroll(32)
            for (int k =0 ; k < UNROLL; ++k) {
                r[k] = fmaf(a[i+j+k*BSIZE], b[i+j+k*BSIZE], r[k]);
            }
        }
    }
    FLOAT_T rs = 0.0;
    for (int i = 0; i < UNROLL; ++i)
        rs += r[i];
    return rs;
}


template<std::size_t TG_N,
         typename    FLOAT_T=float,
         std::size_t VSIZE=100,
         std::size_t BSIZE=5,
         int         UNROLL=5>
FLOAT_T inner_product_parallel_v2(
    int id,
    const FLOAT_T *__restrict a,
    bsg_attr_remote const FLOAT_T *__restrict b)
{
    register FLOAT_T r[UNROLL] = {0.0};

    for (int i = id * BSIZE * UNROLL; i < VSIZE; i += UNROLL * BSIZE * TG_N) {
#pragma bsg_unroll(32)
        for (int j = 0; j < BSIZE; ++j) {
#pragma bsg_unroll(32)
            for (int k =0 ; k < UNROLL; ++k) {
                r[k] = fmaf(a[i+j+k*BSIZE], b[i+j+k*BSIZE], r[k]);
            }
        }
    }
    FLOAT_T rs = 0.0;
    for (int i = 0; i < UNROLL; ++i)
        rs += r[i];

    return rs;
}

template <std::size_t TG_X, std::size_t TG_Y>
class InnerProductParallel_v1 {
public:
    static constexpr std::size_t VSIZE = 100;
    static constexpr std::size_t TG_N = TG_X * TG_Y;
    static constexpr int SYNC_DONE = -2;
    static constexpr int SYNC_INV  = -1;

    InnerProductParallel_v1(bsg_attr_remote const float *t1, const float *t2) {
        _inf = INFINITY;
        for (int i = 0; i < TG_N; ++i)
            _partial[i] = _inf;

        for (int x = 0; x < TG_X; ++x)
            for (int y = 0; y < TG_Y; ++y)
                _t1_idx_group[bsg_x_y_to_id(x,y)]
                    = bsg_tile_group_remote_pointer(x,y,&_t1_idx);

        _t1 = t1;
        _t2 = t2;
        _t1_idx = SYNC_INV;
    }

    void init() {
        if (__bsg_id == 0) {
            return;
        }

        float p = 0.0;
        int t1_idx;
        float *partial_result = bsg_tile_group_remote_pointer(0, 0, &_partial[__bsg_id]);

        while (true) {
            t1_idx = sleep_until_valid(&_t1_idx, SYNC_INV);
            if (t1_idx == SYNC_DONE)
                break;

            p = inner_product_parallel_v1<TG_X,TG_Y>(_t2, &_t1[t1_idx * VSIZE]);
            *partial_result = p;
        }
    }

    float inner_product(int idx) {
        if (__bsg_id != 0)
            return 0.0;

        for (int tile = 0; tile < TG_X*TG_Y; ++tile)
            *_t1_idx_group[tile] = idx;

        _partial[__bsg_id] = inner_product_parallel_v1<TG_X,TG_Y>(_t2, &_t1[idx * VSIZE]);

        float r = 0.0;
        for (int tile = 0; tile <TG_X*TG_Y; ++tile) {
            float tmp = sleep_until_valid(&_partial[tile], _inf);
            r += tmp;
        }

        return r;
    }

    void exit() {
        if (__bsg_id != 0)
            return;

        for (int tile = 0; tile < TG_X*TG_Y; ++tile)
            *_t1_idx_group[tile] = SYNC_DONE;

        return;
    }

    bsg_attr_remote const float   *_t1;
    const float                   *_t2;
    int                            _t1_idx;
    int                           *_t1_idx_group[TG_N];
    float                          _partial[TG_N];
    float                          _inf;
};

template <std::size_t TG_Y>
class InnerProductParallel_Y {
public:
    static constexpr std::size_t VSIZE = 100;
    static constexpr int SYNC_DONE = -2;
    static constexpr int SYNC_INV  = -1;

    InnerProductParallel_Y(bsg_attr_remote const float *t1, const float *t2) {        
        _inf = INFINITY;
        for (int i = 0; i < TG_Y; ++i)
            _partial[i] = _inf;

        for (int y = 0; y < TG_Y; ++y)
            _t1_idx_group[y] = bsg_tile_group_remote_pointer(__bsg_x, y, &_t1_idx);        

        _t1 = t1;
        _t2 = t2;
        _t1_idx = SYNC_INV;
    }

    void init() {
        if (__bsg_y == 0) {
            return;
        }

        float p = 0.0;
        int t1_idx;
        float *partial_result = bsg_tile_group_remote_pointer(__bsg_x, 0, &_partial[__bsg_y]);

        while (true) {
            t1_idx = sleep_until_valid(&_t1_idx, SYNC_INV);
            if (t1_idx == SYNC_DONE)
                break;

            p = inner_product_parallel_v2<TG_Y>(__bsg_y, _t2, &_t1[t1_idx * VSIZE]);
            *partial_result = p;
        }
    }

    float inner_product(int idx) {
        if (__bsg_y != 0)
            return 0.0;

        for (int tile = 0; tile < TG_Y; ++tile)
            *_t1_idx_group[tile] = idx;

        _partial[__bsg_y] = inner_product_parallel_v2<TG_Y>(__bsg_y, _t2, &_t1[idx * VSIZE]);

        float r = 0.0;
        for (int tile = 0; tile <TG_Y; ++tile) {
            float tmp = sleep_until_valid(&_partial[tile], _inf);
            r += tmp;
        }

        return r;
    }

    void exit() {
        if (__bsg_y != 0)
            return;

        for (int tile = 0; tile < TG_Y; ++tile)
            *_t1_idx_group[tile] = SYNC_DONE;

        return;
    }

    bsg_attr_remote const float   *_t1;
    const float                   *_t2;
    int                            _t1_idx;
    int                           *_t1_idx_group [TG_Y];
    float                          _partial      [TG_Y];
    float                          _inf;
};

