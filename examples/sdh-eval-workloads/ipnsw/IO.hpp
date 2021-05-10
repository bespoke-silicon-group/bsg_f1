#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <Graph.hpp>
#include <Graph500Data.hpp>
#include <StringHelpers.hpp>
#include <sstream>
#include <map>

namespace ipnsw {
    //using graph_tools::Graph;
    //using graph_tools::Graph500Data;

    class Parser {
    public:
        using OptionTable = std::map<std::string, std::string>;

        Parser(){}

        void parse(int argc, char *argv[]) {
            int pos = 0;
            int arg = 0;

            while (arg < argc) {
                std::string argstr = std::string(argv[arg]);
                if (ipnsw::startswith(argstr, "--")) {
                    // optional argument
                    if (++arg >= argc) {
                        throw std::runtime_error("'" + argstr + "' requries an argument");
                    }
                    _options[argstr] = std::string(argv[arg]);

                } else {
                    // positional argument
                    switch (pos++) {
                    case 0:
                        _exe = argstr;
                        break;

                    case 1:
                        _ucode = argstr;
                        break;

                    case 2:
                        _version = argstr;
                        break;

                    case 3:
                        _data = argstr;
                        break;

                    case 4:
                        _queries = argstr;
                        break;

                    case 5:
                    case 6:
                    case 7:
                    case 8:
                        _graphs.push_back(argstr);
                        break;

                    default:
                        break;
                    }
                }
                arg++;
            };

            // _exe = std::string(argv[0]);
            // _ucode = std::string(argv[1]);
            // _version = std::string(argv[2]);
            // _data = std::string(argv[3]);
            // _queries = std::string(argv[4]);
            // // graphs
            // for (int i = 5; i < argc; ++i) {
            //     _graphs.push_back(std::string(argv[i]));
            // }
        }

        std::string str() const {
            std::stringstream ss;
            ss << "ucode: " << _ucode << "\n"
               << "version: " << _version << "\n"
               << "exe: " << _exe << "\n"
               << "data: " << _data << "\n"
               << "queries: " << _queries << "\n";

            for (int i = 0; i < _graphs.size(); ++i) {
                ss << "graph " << i << ": " << _graphs[i] << "\n";
            }

            return ss.str();
        }

        std::string option(const std::string &opt) const {
            auto it = _options.find(opt);
            if (it != _options.end())
                return it->second;

            return "";
        }

        std::vector<int> do_queries() const {
            std::string do_queries_str = option("--queries");
            if (do_queries_str.empty()) {
                return {};
            }

            std::vector<int> _do_queries;
            size_t pos = 0;
            size_t at = 0;

            while ((at = do_queries_str.find(",", pos)) != std::string::npos) {
                    do_queries_str.replace(at, 1, " ");
                    pos = at+1;
            }

            std::stringstream ss(do_queries_str);
            while (ss.good()) {
                int q;
                ss >> q;
                _do_queries.push_back(q);
            }

            return _do_queries;
        }

        int num_iproducts() const {
            int n = 100;
            auto s = option("--num-iproducts");
            if (!s.empty()) {
                n = from_string<int>(s);
            }
            return n;
        }

        int grid_x() const {
            auto s = option("--grid-x");
            if (!s.empty())
                return from_string<int>(s);
            else
                return 1;
        }

        int grid_y() const {
            auto s = option("--grid-y");
            if (!s.empty())
                return from_string<int>(s);
            else
                return 1;
        }

        int grp_x() const {
            auto s = option("--group-x");
            if (!s.empty())
                return from_string<int>(s);
            else
                return 1;
        }

        int grp_y() const {
            auto s = option("--group-y");
            if (!s.empty())
                return from_string<int>(s);
            else
                return 1;
        }

        std::string ucode() const   { return _ucode; }
        std::string version() const { return _version; }
        std::string exe() const     { return _exe; }
        std::vector<std::string> graphs() const { return _graphs; }
        std::string graph(int i) const { return _graphs[i]; }
        std::string data() const    { return _data; }
        std::string queries() const { return _queries; }

        std::string              _ucode;
        std::string              _version;
        std::string              _exe;
        std::vector<std::string> _graphs;
        std::string              _data;
        std::string              _queries;
        OptionTable              _options;
    };

    class IO {
    public:
        IO() {}
        IO(const Parser &p): _parser(p) {}


        graph_tools::Graph graph(int i) {
            std::cout << "Reading graph " << i << ": "
                      << _parser._graphs[i] << std::endl;

            graph_tools::Graph500Data d = graph_tools::Graph500Data::FromASCIIFile(_parser._graphs[i]);
            return graph_tools::Graph::FromGraph500Data(d);
        }

        std::vector<graph_tools::Graph> graphs() {
            std::vector<graph_tools::Graph> graphs;
            for (int i = 0; i < _parser._graphs.size(); ++i)
                graphs.push_back(graph(i));

            return graphs;
        }

        template <typename T>
        std::vector<T> read(const std::string & fname) {
            int r;
            struct stat st;

            std::cerr << "Opening " << fname << std::endl;

            r = stat(fname.c_str(), &st);
            if (r != 0) {
                auto s = fname + ": " + std::string(strerror(errno));
                throw std::runtime_error(s);
            }
            std::vector<T> v(st.st_size/sizeof(T));

            FILE *f = fopen(fname.c_str(), "rb");
            if (!f) {
                auto s = fname + ": " + std::string(strerror(errno));
                throw std::runtime_error(s);
            }

            fread(&v[0], st.st_size, 1, f);
            fclose(f);
            return v;
        }

        template <typename T, int N>
        std::vector<std::array<T, N>>
        database() {
            using array = std::array<T,N>;
            return read<array>(_parser._data);
        }

        template <typename T, int N>
        std::vector<std::array<T, N>>
        queries() {
            using array = std::array<T,N>;
            return read<array>(_parser._queries);
        }

        std::string ucode() const { return _parser._ucode; }
        std::vector<int> do_queries() const  { return _parser.do_queries(); }

        Parser _parser;
    };

}
