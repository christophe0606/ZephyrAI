from cmsis_stream.cg.scheduler import GenericNode,F32,Q15
from .NodeTypes import *


class InterleaveStereo(GenericNode):
    def __init__(self,name,theType,outLength):
        GenericNode.__init__(self,name,identified=False)
        if theType == F32 or theType == F32_SCALAR:
            inputType = F32_SCALAR
            outputType = F32_STEREO
        elif theType == Q15 or theType == Q15_SCALAR:
            inputType = Q15_SCALAR
            outputType = Q15_STEREO
        else:
            raise ValueError("Unsupported type for InterleaveStereo: {}".format(theType))
        self.addInput("l",inputType,outLength)
        self.addInput("r",inputType,outLength)
        self.addOutput("o",outputType,outLength)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "InterleaveStereo"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"