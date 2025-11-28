from cmsis_stream.cg.scheduler import GenericSource

from .NodeTypes import *

class ZephyrAudioSource(GenericSource):
    COUNT = 0
    def __init__(self,name,outLength,master=True):
        GenericSource.__init__(self,name)
        ZephyrAudioSource.COUNT = ZephyrAudioSource.COUNT + 1
        if (ZephyrAudioSource.COUNT > 1):
            raise Exception("Only one ZephyrAudioSource node can be instantiated")
        # Stereo output
        self.addOutput("o",Q15_STEREO,outLength)
        self.addLiteralArg(1 if master else 0)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "ZephyrAudioSource"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"