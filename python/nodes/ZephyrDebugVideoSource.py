from cmsis_stream.cg.scheduler import GenericSource

from .NodeTypes import *

class ZephyrDebugVideoSource(GenericSource):
    def __init__(self,name):
        GenericSource.__init__(self,name,identified=True)
        # Stereo output
        self.addEventOutput()

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "ZephyrDebugVideoSource"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"