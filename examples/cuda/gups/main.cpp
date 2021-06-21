#define DEBUG
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
#include <bsg_manycore_printing.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <bsg_manycore_regression.h>
#include "HammerBlade.hpp"
#include "gups/CL.hpp"
#include <vector>
#include <algorithm>

#define TABLE_SIZE (64<<10)
#define UPDATES_PER_CORE 16
#define CORES 1
#define UPDATES ((CORES)*(UPDATES_PER_CORE))

using namespace hammerblade::host;
using namespace BFS;
using namespace std;

HammerBlade::Ptr HB;

static std::vector<int> setup_X()
{
    std::uniform_int_distribution<int> dist(0, TABLE_SIZE);
    std::default_random_engine gen;
    
    std::vector<int> X(UPDATES);
    for (int i = 0; i < X.size(); i++)
        X[i] = dist(gen);

    return X;
}


static std::vector<int> setup_A()
{
    std::vector<int> A(TABLE_SIZE);
    for (int i = 0; i < A.size(); i++)
        A[i] = i;

    return A;
}

int Main(int argc, char *argv[])
{
    CL cl;
    cl.parse(argc, argv);
    
    HB = HammerBlade::Get();
    HB->load_application(cl.binary_path());
    // setup X
    std::vector<int> X = setup_X();    

    // allocate and write X
    hb_mc_eva_t X_dev = HB->alloc(X.size()*sizeof(int));
    HB->push_write(X_dev, &X[0], X.size()*sizeof(int));

    bsg_pr_dbg("Writing X\n");
    HB->sync_write();
    
    // prime X
    bsg_pr_dbg("Priming X\n");
    HB->push_job(Dim(CORES,1), Dim(1,1), "prime", X_dev, UPDATES);
    HB->exec();
    
    // setup A
    std::vector<int> A = setup_A();
    
    // allocate and write A (also invalidates cache)
    hb_mc_eva_t A_dev = HB->alloc(A.size()*sizeof(int));
    HB->push_write(A_dev, &A[0], A.size()*sizeof(int));
    bsg_pr_dbg("Writing A\n");
    HB->sync_write();
    
    // run GUPS
    bsg_pr_dbg("Running %d updates with %d groups (%d per group)\n",
               UPDATES, CORES, UPDATES_PER_CORE);

    HB->push_job(Dim(CORES,1), Dim(1,1), "gups", A_dev);
    HB->exec();

    return HB_MC_SUCCESS;
}

declare_program_main("GUPS", Main);
