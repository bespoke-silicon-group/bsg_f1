#pragma once
#ifndef __PR_PULL_BENCHMARK_HPP
#define __PR_PULL_BENCHMARK_HPP

#include "hb_intrinsics.h" //graphit host runtime libs
#include "infra_hb/host/arg_parser.hpp"
#include <bsg_manycore_regression.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream> 
#include <atomic>
#include <bsg_manycore_tile.h>
#include <bsg_manycore_errno.h>
#include <bsg_manycore_tile.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_cuda.h>


using hammerblade::Device;
using hammerblade::Vector;
using hammerblade::GraphHB;
using hammerblade::GlobalScalar;

#endif
