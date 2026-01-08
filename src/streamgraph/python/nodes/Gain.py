from cmsis_stream.cg.scheduler import GenericNode
from .NodeTypes import *


class Gain(GenericNode):
    def __init__(self,name,theType,outLength,gain=1.0):
        GenericNode.__init__(self,name)
        self.addInput("i",theType,outLength)
        self.addOutput("o",theType,outLength)
        self.addLiteralArg(gain)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Gain"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"