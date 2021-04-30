#pragma once
#ifdef __cplusplus
extern "C" {
#endif

// To make your program HammerBlade cross-platform compatible,
// define a function with the signature of "main", and then
// use this macro to mark it as the entry point of your program
//
// Example:
//
//    int MyMain(int argc, char *argv[]) {
//        /* your code here */
//    }
//    declare_program_main("The name of your test", MyMain)
//
#define declare_program_main(test_name, name)                   \
    int main(int argc, char *argv[]) {                          \
        bsg_pr_test_info("Regression Test: %s\n", test_name);   \
        int rc = name(argc, argv);                              \
        bsg_pr_test_pass_fail(rc == HB_MC_SUCCESS);             \
        return rc;                                              \
    }

#ifdef __cplusplus
}
#endif

// Some library functions need to be compiled in both RISC-V
// and x86. Makefile scripts don't recompile a target that was already
// executed even if the file does not exist. Therefore such functions
// are defined as macros here which are included in
// bsg_manycore_simulator.cpp
#define declare_hb_mc_get_bits                                            \
  uint32_t hb_mc_get_bits (uint32_t val, uint32_t start, uint32_t size) { \
    uint32_t mask = ((1 << size) - 1) << start;                           \
    return ((val & mask) >> start);                                       \
  }                                                                       \

