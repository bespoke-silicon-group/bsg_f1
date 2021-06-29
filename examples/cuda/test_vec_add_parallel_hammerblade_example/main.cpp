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

#include <bsg_manycore_tile.h>
#include <bsg_manycore_errno.h>
#include <bsg_manycore_tile.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_cuda.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <bsg_manycore_regression.h>
#include "HammerBlade.hpp"

using namespace hammerblade::host;
#define ALLOC_NAME "default_allocator"

/*!
 * Runs the vector addition a grid of 2x2 tile groups. A[N] + B[N] --> C[N]
 * Grid dimensions are determines by how much of a load we want for each tile group (block_size_x)
 * This tests uses the software/spmd/bsg_cuda_lite_runtime/vec_add_parallel/ Manycore binary in the BSG Manycore bitbucket repository.  
*/


void host_vec_add (uint32_t *A, uint32_t *B, uint32_t *C, uint32_t N) { 
        for (uint32_t i = 0; i < N; i ++) { 
                C[i] = A[i] + B[i];
        }
        return;
}

int kernel_vec_add_parallel (int argc, char **argv) {
        int rc;
        char *bin_path, *test_name;
        struct arguments_path args = {NULL, NULL};

        argp_parse (&argp_path, argc, argv, 0, 0, &args);
        bin_path = args.path;
        test_name = args.name;

        bsg_pr_test_info("Running the CUDA Vector Addition Kernel on a grid of 2x2 tile groups.\n\n");

        HammerBlade::Ptr hb = HammerBlade::Get();
        hb->load_application(bin_path);

        uint32_t N = 1024;

        eva_t A_device, B_device, C_device;
        A_device = hb->alloc(N*sizeof(uint32_t));
        B_device = hb->alloc(N*sizeof(uint32_t));
        C_device = hb->alloc(N*sizeof(uint32_t));

        uint32_t A_host[N]; /* allocate A[N] on the host */
        uint32_t B_host[N]; /* allocate B[N] on the host */
        for (uint32_t i = 0; i < N; i++) { /* fill A with arbitrary data */
            A_host[i] = rand() & 0xFFFF;
            B_host[i] = rand() & 0xFFFF;
        }

        hb->push_write(A_device, A_host, sizeof(A_host));
        hb->push_write(B_device, B_host, sizeof(B_host));
        hb->sync_write();
        
        uint32_t block_size_x = 64;

        hb->push_job(Dim(N/block_size_x, 1), Dim(2,2), "kernel_vec_add_parallel", A_device, B_device, C_device, N, block_size_x);
        hb->exec();

        uint32_t C_host[N];
        hb->push_read(C_device, C_host, sizeof(C_host));
        hb->sync_read();

        uint32_t C_expected[N];
        host_vec_add (A_host, B_host, C_expected, N);
                
        int mismatch = 0;
        for (uint32_t i = 0; i < N; i++) {
            if (A_host[i] + B_host[i] != C_host[i]) {
                bsg_pr_err(BSG_RED("Mismatch: ") "C[%d]:"
                           "  0x%08" PRIx32 " + 0x%08" PRIx32 " = 0x%08" PRIx32 "\t Expected: 0x%08" PRIx32 "\n",
                           i , A_host[i], B_host[i], C_host[i], C_expected[i]);
                mismatch = 1;
            }
        }

        if (mismatch) {
            return HB_MC_FAIL;
        }

        return HB_MC_SUCCESS;
}

declare_program_main("test_vec_add_parallel", kernel_vec_add_parallel);
