from vcache_utils import VCacheStats
from gups_common import GUPSParameters
import sys
import pandas as pd

class GUPSVCacheStats(VCacheStats):
    def _subclass_init_add_group_by_fields(self):
        self._parameters = GUPSParameters(self.filename)
        self._parameters.updateDataFrame(self._data)
        self._group_by_fields += self._parameters.parameters
        return

data = pd.DataFrame()
for filename in sys.argv[1:]:
    data = data.append(GUPSVCacheStats(filename).diffed_data)

data.to_csv("vcache.summary.csv")
