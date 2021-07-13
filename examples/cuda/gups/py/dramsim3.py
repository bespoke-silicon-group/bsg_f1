from dramsim3_utils import DRAMSim3Stats
import sys
import re
import pandas as pd
from gups_common import GUPSParameter

class GUPSDRAMSim3Stats(DRAMSim3Stats):
    def _subclass_init_add_group_by_fields(self):
        """
        Parse the experiment parameters from the filename
        """
        self._parameters = GUPSParameter(self.filename)
        self._data = self._parameters.updateDataFrame(self._data)
        self._group_by_fields += self._parameters.parameters
        return

df = pd.DataFrame()
for f in sys.argv[1:]:
    df = df.append(GUPSDRAMSim3Stats(f).diffed_data)

df.to_csv('dramsim3.summary.csv')

