from cmsis_stream.cg.scheduler import GenericNode,F32,Q15
from .NodeTypes import *


class DeinterleaveStereo(GenericNode):
    def __init__(self,name,theType,outLength):
        GenericNode.__init__(self,name,identified=False)
        if theType == F32 or theType == F32_STEREO:
            inputType = F32_STEREO
            outputType = F32_SCALAR
        elif theType == Q15 or theType == Q15_STEREO:
            inputType = Q15_STEREO
            outputType = Q15_SCALAR
        else:
            raise ValueError("Unsupported type for DeinterleaveStereo: {}".format(theType))
        self.addInput("i",inputType,outLength)
        self.addOutput("l",outputType,outLength)
        self.addOutput("r",outputType,outLength)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "DeinterleaveStereo"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"