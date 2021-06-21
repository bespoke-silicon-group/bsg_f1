#include <string>
namespace GUPS {
    class CL {
    public:
        CL() {}
        int parse(int argc, char *argv[]) {
            _exec_path = argv[0];
            _binary_path = argv[1];
            _kernel_name = argv[2];
            _table_size = atoi(argv[3]);
            _updates_per_core = atoi(argv[4]);
            _cores = atoi(argv[5]);
            return 0;
        }

        std::string _binary_path;
        std::string _exec_path;
        std::string _kernel_name;
        int         _table_size;
        int         _updates_per_core;
        int         _cores;

        std::string binary_path() const { return _binary_path; }
        std::string exec_path() const { return _exec_path; }
        std::string kernel_name() const { return _kernel_name; }
        int table_size() const { return _table_size; }
        int updates_per_core() const { return _updates_per_core; }
        int cores() const { return _cores; }
        int updates() const { return updates_per_core() * cores(); }
    };
}
