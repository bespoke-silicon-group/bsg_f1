import pandas
import re
class GUPSParameters(object):
    def __init__(self, filename):
        """
        Parse the experiment parameters from the filename
        """
        self._filename = filename
        m = re.search(r'(\d+)_table-words__(\d+)_updates-per-core__(\d+)_cores__(\d+)_concurrency', self.filename)
        self._table_words = int(m.group(1))
        self._updates_per_core = int(m.group(2))
        self._cores = int(m.group(3))
        self._concurrency = int(m.group(4))

    def updateDataFrame(self, df):
        """
        Adds parameters to data frame as a column
        """
        df["table_words"] = self._table_words
        df["updates_per_core"] = self._updates_per_core
        df["cores"] = self._cores
        df["concurrency"] = self._concurrency
        # return the dataframe
        return df
    
    @property
    def filename(self):
        return self._filename

    @property
    def table_words(self):
        return self._table_words

    @property
    def updates_per_core(self):
        return self._updates_per_core

    @property
    def cores(self):
        return self._cores

    @property
    def concurrency(self):
        return self._concurrency    

    @property
    def parameters(self):
        """
        Return a list of parameters
        """
        return [
            'table_words',
            'updates_per_core',
            'cores',
            'concurrency',
        ]
