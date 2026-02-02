from cmsis_stream.cg.scheduler import GenericSource

from .NodeTypes import *

class ZephyrAudioSource(GenericSource):
    def __init__(self,name,outLength):
        GenericSource.__init__(self,name,identified=True)
        # Stereo output
        self.addOutput("o",Q15_STEREO,outLength)
        # hw_ is common to all node and does not name a specific node
        self.addVariableArg(f"params->hw_")

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "ZephyrAudioSource"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"