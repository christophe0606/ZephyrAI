from cmsis_stream.cg.scheduler import GenericSource

from .NodeTypes import *

class ZephyrVideoSource(GenericSource):
    COUNT = 0
    def __init__(self,name):
        GenericSource.__init__(self,name)
        ZephyrVideoSource.COUNT = ZephyrVideoSource.COUNT + 1
        if (ZephyrVideoSource.COUNT > 1):
            raise Exception("Only one ZephyrVideoSource node can be instantiated")
        # Stereo output
        self.addEventOutput()

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "ZephyrVideoSource"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"