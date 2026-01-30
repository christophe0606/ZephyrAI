from cmsis_stream.cg.scheduler import GenericNode
from .NodeTypes import *

class StereoToMono(GenericNode):
    def __init__(self,name,theType,ioLength):
        GenericNode.__init__(self,name,identified=False)
        self.addInput("l",theType,ioLength)
        self.addInput("r",theType,ioLength)
        self.addOutput("o",theType,ioLength)

    @property
    def typeName(self):
        return "StereoToMono"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"