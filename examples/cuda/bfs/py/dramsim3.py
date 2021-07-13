from dramsim3_utils import DRAMSim3Stats
import sys
import re
import pandas as pd
from bfs_common import BFSParameters

class BFSDRAMSim3Stats(DRAMSim3Stats):
    def _subclass_init_add_group_by_fields(self):
        """
        Parse the experiment parameters from the filename
        """
        # parse the parameters
        self._parameters = BFSParameters(self.filename)

        # add parameters to dataframe
        self._data = self._parameters.updateDataFrame(self._data)

        # append these fields to the group_by_fields
        self._group_by_fields += self._parameters.parameters


df = pd.DataFrame()

for f in sys.argv[1:]:
    df = df.append(BFSDRAMSim3Stats(f).diffed_data)

df.to_csv('dramsim3.summary.csv')

