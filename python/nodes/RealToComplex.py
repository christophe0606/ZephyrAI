from cmsis_stream.cg.scheduler import GenericNode,F32,Q15
from .NodeTypes import *


class RealToComplex(GenericNode):
    def __init__(self,name,theType,outLength):
        GenericNode.__init__(self,name,identified=False)
        if theType == F32:
            inputType = F32_SCALAR
            outputType = F32_COMPLEX
        elif theType == Q15:
            inputType = Q15_SCALAR
            outputType = Q15_COMPLEX
        else:
            raise ValueError("Unsupported type for RealToComplex: {}".format(theType))
        self.addInput("i",inputType,outLength)
        self.addOutput("o",outputType,outLength)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "RealToComplex"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"