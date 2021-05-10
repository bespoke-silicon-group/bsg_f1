#pragma once
#include <bsg_manycore.h>
template <typename T>
static inline T sleep_on_update(volatile T *ptr)
{
    T r;
    asm volatile ("lr.w.aq %[r], %[ptr]" :
                  [r]   "=r" (r) :
                  [ptr] "m"  (*ptr)
        );
    return r;
}

template <typename T>
static inline T sleep_until_valid(volatile T *ptr, T not_valid)
{
    T r;

    asm volatile ("lr.w %[r], %[ptr]" :
                  [r] "=r" (r) :
                  [ptr] "m" (*ptr));

    while (r == not_valid) {
        r = sleep_on_update(ptr);
    }
    *ptr = not_valid;
    return r;
}
