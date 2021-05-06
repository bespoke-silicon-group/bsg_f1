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

#define declare_bsg_printing                                                          \
  typedef struct prefix_info {                                                        \
    FILE *file;                                                                       \
    bool newline;                                                                     \
    void (*newline_hook)(struct prefix_info *info, const char *prefix);               \
} prefix_info_t;                                                                      \
                                                                                      \
/* inserts the time into the prefix */                                                \
static void insert_time(prefix_info_t *info, const char *prefix)                      \
{                                                                                     \
        fprintf(info->file, "%s @ (%llu): ", prefix, bsg_utc());                      \
        return;                                                                       \
}                                                                                     \
                                                                                      \
typedef std::map<std::string, prefix_info_t> prefix_map;                              \
                                                                                      \
static prefix_map fmap = {                                                            \
  {BSG_PRINT_PREFIX_DEBUG, {BSG_PRINT_STREAM_DEBUG, true, insert_time}},              \
  {BSG_PRINT_PREFIX_ERROR, {BSG_PRINT_STREAM_ERROR, true, 0}},                        \
  {BSG_PRINT_PREFIX_WARN,  {BSG_PRINT_STREAM_WARN,  true, 0}},                        \
  {BSG_PRINT_PREFIX_INFO,  {BSG_PRINT_STREAM_INFO,  true, 0}},                        \
};                                                                                    \
                                                                                      \
int bsg_pr_prefix(const char *prefix, const char *fmt, ...)                           \
{                                                                                     \
  std::string prefix_string(prefix);                                                  \
  std::string fmt_string(fmt);                                                        \
  prefix_info_t *info;                                                                \
  va_list ap;                                                                         \
  int r = -1, count = 0;                                                              \
  bool newline;                                                                       \
                                                                                      \
  auto it = fmap.find(prefix_string);                                                 \
  if (it != fmap.end()) {                                                             \
          info = &it->second;                                                         \
  } else {                                                                            \
          return r;                                                                   \
  }                                                                                   \
                                                                                      \
  /* lock our file to make our print atomic */                                        \
  flockfile(info->file);                                                              \
  va_start(ap, fmt);                                                                  \
  newline = info->newline;                                                            \
                                                                                      \
  std::string::size_type  cl_start = 0, cl_end;                                       \
  do { /* for each line in fmt */                                                     \
          /* find end of line */                                                      \
          cl_end = fmt_string.find('\n', cl_start);                                   \
          cl_end = (cl_end == std::string::npos ? fmt_string.size() : cl_end);        \
                                                                                      \
          /* print prefix if this is the start of a new line */                       \
          if (newline) {                                                              \
                  if (info->newline_hook) {                                           \
                          info->newline_hook(info, prefix);                           \
                  } else {                                                            \
                          fprintf(info->file, "%s", prefix);                          \
                  }                                                                   \
          }                                                                           \
                                                                                      \
          /* print to the end of the line */                                          \
          std::string lfmt = fmt_string.substr(cl_start, cl_end-cl_start);            \
                                                                                      \
          count += vfprintf(info->file, lfmt.c_str(), ap);                            \
                                                                                      \
          /* move cl_start forward */                                                 \
          cl_start = cl_end;                                                          \
                                                                                      \
          /* decide on newline */                                                     \
          if (cl_end != fmt_string.size()) {                                          \
            /* print the newline character and set 'newline' to true */               \
            count += fprintf(info->file, "\n");                                       \
            cl_start++; /* advance one so we don't print the 'newline' twice */       \
            newline = true;                                                           \
          } else {                                                                    \
            /* this was the last line printed */                                      \
            /* if it was an empty line just set newline to false */                   \
            newline = lfmt.empty() ? true : false;                                    \
          }                                                                           \
                                                                                      \
          /* until we've reached the end of our format string */                      \
  } while (cl_start < fmt_string.size());                                             \
                                                                                      \
  /* success */                                                                       \
  r = count;                                                                          \
                                                                                      \
 exit_func:                                                                           \
        /* setup for the next call */                                                 \
        info->newline = newline;                                                      \
        va_end(ap);                                                                   \
        funlockfile(info->file);                                                      \
        return r;                                                                     \
}                                                                                     \
