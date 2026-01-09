from cmsis_stream.cg.scheduler import GenericSource

from .NodeTypes import *

class ZephyrDebugAudioSource(GenericSource):
    COUNT = 0
    def __init__(self,name,outLength):
        GenericSource.__init__(self,name)
        ZephyrDebugAudioSource.COUNT = ZephyrDebugAudioSource.COUNT + 1
        if (ZephyrDebugAudioSource.COUNT > 1):
            raise Exception("Only one ZephyrDebugAudioSource node can be instantiated")
        # Stereo output
        self.addOutput("o",Q15_STEREO,outLength)
        self.addEventOutput()

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "ZephyrDebugAudioSource"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"