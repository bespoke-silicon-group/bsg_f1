TEST_GRAPH_TYPE = graph500
TEST_VERTICES = $(shell echo   1*1024*1024|bc)
TEST_EDGES    = $(shell echo  16*1024*1024|bc)
TEST_ROOT_NODE    = 182
TEST_ITERS        = 0 1 2 3 4 5 6 7 8 9 10
