from cmsis_stream.cg.scheduler import GenericNode

from ..nodes.NodeTypes import F32_SCALAR

class Mixer(GenericNode):
    def __init__(self,name,outLength):
        GenericNode.__init__(self,name,identified=False)
        self.addInput("inl",F32_SCALAR,outLength)
        self.addInput("inr",F32_SCALAR,outLength)
        self.addOutput("oul",F32_SCALAR,outLength)
        self.addOutput("our",F32_SCALAR,outLength)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Mixer"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "appnodes"