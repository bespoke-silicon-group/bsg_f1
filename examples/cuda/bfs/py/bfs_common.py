import pandas
import re
class BFSParameters(object):
    def __init__(self, filename):
        """
        Parse the experiment parameters from the filename
        """
        self._filename = filename
        m = re.search(r'(graph500|uniform)_graph-type__(\d+)_vertices__(\d+)_edges__(\d+)_root__'
                      + r'(\d+)_iter__(\d+)_tile-groups__(\d+)_tgx__(\d+)_tgy',
                      self.filename)
        
        self._graph_type = m.group(1)
        self._vertices = int(m.group(2))
        self._edges = int(m.group(3))
        self._root = int(m.group(4))
        self._iter = int(m.group(5))
        self._tile_groups = int(m.group(6))
        self._tgx = int(m.group(7))
        self._tgy = int(m.group(8))

    def updateDataFrame(self, df):
        """
        Adds parameters to data frame as a column
        """
        df['graph_type'] = self.graph_type
        df['vertices'] = self.vertices
        df['edges'] = self.edges
        df['root'] = self.root
        df['iter'] = self.iter
        df['tile_groups'] = self.tile_groups
        df['tgx'] = self.tgx
        df['tgy'] = self.tgy
        # return the dataframe
        return df
    
    @property
    def filename(self):
        return self._filename

    @property
    def graph_type(self):
        return self._graph_type

    @property
    def vertices(self):
        return self._vertices

    @property
    def edges(self):
        return self._edges

    @property
    def root(self):
        return self._root

    @property
    def iter(self):
        return self._iter

    @property
    def tile_groups(self):
        return self._tile_groups

    @property
    def tgx(self):
        return self._tgx

    @property
    def tgy(self):
        return self._tgy
    

    @property
    def parameters(self):
        """
        Return a list of parameters
        """
        return [
            'graph_type',
            'vertices',
            'edges',
            'root',
            'iter',
            'tile_groups',
            'tgx',
            'tgy'
        ]
