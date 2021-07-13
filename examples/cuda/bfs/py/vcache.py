from vcache_utils import VCacheStats
from bfs_common import BFSParameters
import sys
import pandas as pd

class BFSVCacheStats(VCacheStats):
    def _subclass_init_add_group_by_fields(self):
        self._parameters = BFSParameters(self.filename)
        self._parameters.updateDataFrame(self._data)
        self._group_by_fields += self._parameters.parameters
        return

data = pd.DataFrame()
for filename in sys.argv[1:]:
    data = data.append(BFSVCacheStats(filename).diffed_data)

data.to_csv("vcache.summary.csv")
