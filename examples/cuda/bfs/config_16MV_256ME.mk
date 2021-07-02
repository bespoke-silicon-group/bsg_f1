TEST_GRAPH_TYPE = graph500
TEST_VERTICES = $(shell echo   16*1024*1024|bc)
TEST_EDGES    = $(shell echo  256*1024*1024|bc)
TEST_ROOT_NODE    = 182
TEST_ITERS        = 1 2 6
