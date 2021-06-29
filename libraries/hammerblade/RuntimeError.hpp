#pragma once
#include <bsg_manycore_errno.h>
#include <stdexcept>
#include <iostream>
#include <string>

namespace hammerblade {
    namespace host {
        class RuntimeError : public std::runtime_error {
        public:
            RuntimeError(int err) :
                std::runtime_error("HammerBlade Error: " + std::string(hb_mc_strerror(err))) {}

            static
            void Test() {
                {
                    try {
                        throw RuntimeError(HB_MC_SUCCESS);
                    } catch (RuntimeError e) {
                        std::cout << "Error Message: '" << e.what() << "'" << std::endl;
                    }
                }
            }
        };
    }
}
