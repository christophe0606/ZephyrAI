from cmsis_stream.cg.scheduler import GenericSink
from nodes import *
    
class SendToNetwork(GenericSink):
    def __init__(self,name,theType,nbSamples):
        GenericSink.__init__(self,name,identified=False)
        self.addInput("i",theType,nbSamples)
        self.addEventInput()
        self.addEventOutput()

    @property
    def folder(self):
        """The folder where the C++ implementation of this node is located"""
        return "nodes"
    
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "SendToNetwork"