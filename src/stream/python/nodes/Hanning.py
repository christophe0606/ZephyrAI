from cmsis_stream.cg.scheduler import GenericNode
from .NodeTypes import *


class Hanning(GenericNode):
    def __init__(self,name,outLength):
        GenericNode.__init__(self,name)
        self.addInput("i",F32_SCALAR,outLength)
        self.addOutput("o",F32_SCALAR,outLength)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Hanning"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"