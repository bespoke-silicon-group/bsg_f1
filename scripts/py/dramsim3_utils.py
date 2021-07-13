import sys
import pandas as pd
import numpy as np

def fix_dramsim3_json(filename_i, filename_o):
    jsonfile_o = open(filename_o, "w")
    with open(filename_i, "r") as jsonfile:
        lines = [line for line in jsonfile]

    for (lno, line) in enumerate(lines):
        # if this is the last line
        # replace the ',' with ']'
        if lno == len(lines)-1:
            line = line.replace(',\n',']\n')
        # write it out
        jsonfile_o.write(line)

class DRAMSim3Stats(object):
    """
    wrapper for DRAMSim3 stats object
    """
    def __init__(self, json_filename):
        """
        Initialize from a JSON file from DRAMSim3
        """
        self._filename = json_filename + ".fixed.json"        
        fix_dramsim3_json(json_filename, self._filename)

        # setup the "groupby" fields
        self._group_by_fields = ["tag"]
                
        # read the json file
        self._data = pd.read_json(self._filename)
        self._subclass_init_add_group_by_fields()

        # only keep the min and max cycles
        self._max_cycle = self._data['num_cycles'].max()
        self._min_cycle = self._data['num_cycles'].min()

        filtered_data = self._data[(self._data["num_cycles"]==self._min_cycle)|
                                   (self._data["num_cycles"]==self._max_cycle)]        
        
        filtered_data = filtered_data.drop([
            'act_stb_energy',
            'all_bank_idle_cycles',
            'pre_stb_energy',
            'rank_active_cycles',
            'sref_cycles',
            'sref_energy',            
        ], 1)        
        
        # do a diff by channel
        # bug here! num_cycles gets summed
        self._diffed_data = filtered_data.groupby(self._group_by_fields).sum().diff().dropna()
        self._diffed_data['cycles'] = self._diffed_data['num_cycles']/8

        self._epoch_data = filtered_data.groupby(['num_cycles'] + self._group_by_fields).sum().diff().dropna()
        
    def _subclass_init_add_group_by_fields(self):
        """
        This should be overridden by a subclass, if applicable
        Adds fields by which to group to produce diffed data
        """
        return
    
    def __str__(self):
        """
        Format as string
        """
        return str(self._data)

    @property
    def filename(self):
        return self._filename
    
    @property
    def fields(self):
        """
        Returns the column names in a list
        """
        return self._data.columns.to_list()

    @property
    def data(self):
        return self._data

    @property
    def diffed_data(self):
        return self._diffed_data
    
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
    def updates(self):
        return self.updates_per_core * self.cores

    @property
    def epoch_data(self):
        return self._epoch_data
