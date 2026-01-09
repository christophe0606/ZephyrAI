from cmsis_stream.cg.scheduler import GenericSource

from .NodeTypes import *

class ZephyrDebugVideoSource(GenericSource):
    COUNT = 0
    def __init__(self,name):
        GenericSource.__init__(self,name)
        ZephyrDebugVideoSource.COUNT = ZephyrDebugVideoSource.COUNT + 1
        if (ZephyrDebugVideoSource.COUNT > 1):
            raise Exception("Only one ZephyrDebugVideoSource node can be instantiated")
        # Stereo output
        self.addEventOutput()
        self.addVariableArg("evtQueue")

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "ZephyrDebugVideoSource"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"