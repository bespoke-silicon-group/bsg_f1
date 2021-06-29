#include <string>
namespace BFS {
    class CL {
    public:
        CL() {}
        int parse(int argc, char *argv[]) {
            _exec_path = argv[0];
            _binary_path = argv[1];
            return 0;
        }

        std::string _binary_path;
        std::string _exec_path;

        std::string binary_path() const { return _binary_path; }
        std::string exec_path() const { return _exec_path; }
    };
}
