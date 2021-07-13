// Copyright (c) 2019, University of Washington All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
// Redistributions of source code must retain the above copyright notice, this list
// of conditions and the following disclaimer.
// 
// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
// 
// Neither the name of the copyright holder nor the names of its contributors may
// be used to endorse or promote products derived from this software without
// specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef BSG_MANYCORE_EPA_H
#define BSG_MANYCORE_EPA_H
#include <bsg_manycore_features.h>

#ifdef __cplusplus
#include <cstdint>
#include <cstddef>
#else
#include <stdint.h>
#include <stddef.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

        /**
         * An Endpoint Physical Address. This type uniquely identifies a physical
         * memory address within a manycore endpoint. It is a byte address.
         */
        typedef uint32_t hb_mc_epa_t;
        typedef hb_mc_epa_t epa_t;

        /**
         * Checks alignment of an epa based on data size in bytes.
         * @param[in] epa  epa address
         * @param[in] sz   data size in bytes.
         * @return         HB_MC_SUCCESS if npa is aligned and HB_MC_UNALIGNED if not,
         *                 and HB_MC_INVALID otherwise.
         */
        int hb_mc_manycore_epa_check_alignment(const hb_mc_epa_t *epa, size_t sz);

#define HB_MC_EPA_LOGSZ 18
#define HB_MC_GLOBAL_EPA_LOGSZ 16

#define HB_MC_HOST_EPA_FINISH 0xEAD0
#define HB_MC_HOST_EPA_TIME 0xEAD4
#define HB_MC_HOST_EPA_FAIL 0xEAD8
#define HB_MC_HOST_EPA_STDOUT 0xEADC
#define HB_MC_HOST_EPA_STDERR 0xEAE0
#define HB_MC_HOST_EPA_BRANCH_TRACE 0xEAE4
#define HB_MC_HOST_EPA_PRINT_STAT 0xEA0C
#define EPA_FROM_BASE_AND_OFFSET(base, offset)  \
        (((base)+(offset)))

#ifdef __cplusplus
};
#endif
#endif
