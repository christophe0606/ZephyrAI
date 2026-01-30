from cmsis_stream.cg.scheduler import GenericSource

from .NodeTypes import *

class ZephyrDebugAudioSource(GenericSource):
    def __init__(self,name,outLength):
        GenericSource.__init__(self,name,identified=False)
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