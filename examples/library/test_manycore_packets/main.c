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

#include <inttypes.h>
#include <libgen.h>
#include <bsg_manycore.h>
#include <bsg_manycore_tile.h>
#include <bsg_manycore_printing.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_errno.h>
#include <bsg_manycore_printing.h>

#include <bsg_manycore_regression.h>


//TODO(sasha): Make this const once accessor api is fixed in bsg_manycore_packet
void request_packet_to_array(/*const*/ hb_mc_request_packet_t *pack, /*out*/ uint32_t *arr) {
        *arr++ = hb_mc_request_packet_get_x_dst(pack);
        *arr++ = hb_mc_request_packet_get_y_dst(pack);
        *arr++ = hb_mc_request_packet_get_x_src(pack);
        *arr++ = hb_mc_request_packet_get_y_src(pack);
        *arr++ = hb_mc_request_packet_get_load_id(pack);
        *arr++ = hb_mc_request_packet_get_op   (pack);
        *arr++ = hb_mc_request_packet_get_addr (pack);
        *arr++ = hb_mc_request_packet_get_data (pack);
}

void response_packet_to_array(/*const*/ hb_mc_response_packet_t *pack, /*out*/ uint32_t *arr) {
        //NOTE(sasha): Does not test load_id. Not sure if it should,
        //             comment in hb_mc_response_packet_t says it
        //             is unused
        
        *arr++ = hb_mc_response_packet_get_x_dst(pack);
        *arr++ = hb_mc_response_packet_get_y_dst(pack);
        *arr++ = hb_mc_response_packet_get_load_id(pack);
        //        *arr++ = hb_mc_response_packet_get_op   (pack); No encoding for response op in C++
        *arr++ = hb_mc_response_packet_get_data (pack);
}

int test_manycore_packets(int argc, char *argv[]) {
        hb_mc_manycore_t manycore = {0}, *mc = &manycore;
        int err;

        /*****************************/
        /* Initializing the manycore */
        /*****************************/
        err = hb_mc_manycore_init(mc, "test_manycore_packets", 0);
        if (err != HB_MC_SUCCESS) {
                bsg_pr_err(BSG_RED("failed to initialize manycore: %s\n"), hb_mc_strerror(err));
                return err;
        }

        /*************************/
        /* Seeding the test data */
        /*************************/
        srand(time(0));
        
        hb_mc_coordinate_t host = hb_mc_manycore_get_host_coordinate(mc);
        uint32_t host_x = hb_mc_coordinate_get_x(host);
        uint32_t host_y = hb_mc_coordinate_get_y(host);

        uint8_t target_x = hb_mc_config_get_vcore_base_x(hb_mc_manycore_get_config(mc));
        uint8_t target_y = hb_mc_config_get_vcore_base_y(hb_mc_manycore_get_config(mc));
        uint32_t addr = HB_MC_TILE_EPA_DMEM_BASE >> 2; // EPA
        uint32_t data  = rand();
        uint32_t load_id = 31;

        bsg_pr_test_info("Address: 0x%08" PRIx32 ", Expected data: %08" PRIx32 "\n", addr, data);

        hb_mc_request_packet_t req1, req2;
        hb_mc_response_packet_t res;

        const char *req_desc[] = {
                "Destination X", "Destination Y",
                "Source X",      "Source Y",
                "Load ID",        "Opcode",
                "Address",          "Data",
        };

        uint32_t req_expected[] = {
                target_x,                       target_y,
                host_x,                         host_y,
                load_id,     HB_MC_PACKET_OP_REMOTE_SW,
                addr,                           data,
        };

        const char *res_desc[] = {
                "Destination X", "Destination Y",
                "Load ID",
                "Opcode",          "Data",
        };

        uint32_t res_expected[] = {host_x, host_y, load_id,
                /*HB_MC_PACKET_OP_REMOTE_LOAD,*/   data,
        };

        uint32_t actual[8];

        /**************************/
        /* Test packet formatting */
        /**************************/
        bsg_pr_test_info("Manually formatting packet\n");
        hb_mc_request_packet_set_x_dst(&req1, target_x);
        hb_mc_request_packet_set_y_dst(&req1, target_y);
        hb_mc_request_packet_set_x_src(&req1, host_x);
        hb_mc_request_packet_set_y_src(&req1, host_y);
        hb_mc_request_packet_set_data (&req1, data);
        hb_mc_request_packet_set_load_id(&req1, load_id);
        hb_mc_request_packet_set_op   (&req1, HB_MC_PACKET_OP_REMOTE_SW);
        hb_mc_request_packet_set_addr (&req1, addr);

        request_packet_to_array(&req1, actual);
        if(hb_mc_compare_results(8, req_desc, req_expected, actual) == HB_MC_FAIL)
                return HB_MC_FAIL;

        /************************************/
        /* Test transmitting a store packet */
        /************************************/
        bsg_pr_test_info("Sending store packet to tile (%d, %d)\n", target_x, target_y);
        err = hb_mc_manycore_packet_tx(mc, (hb_mc_packet_t*)&req1, HB_MC_MMIO_FIFO_TO_DEVICE, -1);
        if (err != HB_MC_SUCCESS) {
                bsg_pr_test_info(BSG_RED("Failed to write to FIFO (%s)") ": %s\n",
                                 hb_mc_direction_to_string(HB_MC_MMIO_FIFO_TO_DEVICE),
                                 hb_mc_strerror(err));
                return HB_MC_FAIL;
        }
        
        bsg_pr_test_info(BSG_GREEN("Succesfully Wrote packet to FIFO (%s)") "\n",
                         hb_mc_direction_to_string(HB_MC_MMIO_FIFO_TO_DEVICE));

        /************************************************************/
        /* Test transmitting a load packet and receiving the result */
        /************************************************************/
        hb_mc_request_packet_set_op(&req1, HB_MC_PACKET_OP_REMOTE_LOAD);
        hb_mc_request_packet_load_info_t load_info = {{0}};
        hb_mc_request_packet_set_load_info(&req1, load_info);
        bsg_pr_test_info("Testing reading packet\n");
        err = hb_mc_manycore_packet_tx(mc, (hb_mc_packet_t*)&req1, HB_MC_MMIO_FIFO_TO_DEVICE, -1);      
        if(err != HB_MC_SUCCESS) {
                bsg_pr_test_info(BSG_RED("Failed to write to FIFO (%s)") ": %s\n",
                                 hb_mc_direction_to_string(HB_MC_MMIO_FIFO_TO_DEVICE),
                                 hb_mc_strerror(err));
                return HB_MC_FAIL;
        }
        
        bsg_pr_test_info("Reading\n");
        err = hb_mc_manycore_packet_rx(mc, (hb_mc_packet_t*)&res, HB_MC_MMIO_FIFO_TO_DEVICE, -1);
        if (err != HB_MC_SUCCESS) {
                bsg_pr_test_info(BSG_RED("Failed to read from FIFO (%s)") ": %s\n",
                                 hb_mc_direction_to_string(HB_MC_MMIO_FIFO_TO_DEVICE),
                                 hb_mc_strerror(err));
                return HB_MC_FAIL;
        }
        bsg_pr_test_info("Comparing to expected read packet:\n");

        response_packet_to_array(&res, actual);
        if(hb_mc_compare_results(4, res_desc, res_expected, actual) == HB_MC_FAIL)
                return HB_MC_FAIL;

        /*******/
        /* END */
        /*******/
        hb_mc_manycore_exit(mc);
        return HB_MC_SUCCESS;   
}

declare_program_main("test_manycore_packets", test_manycore_packets);
