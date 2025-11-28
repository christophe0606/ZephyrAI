from cmsis_stream.cg.scheduler import GenericSink

from nodes import *

class KWS(TFLite):
    def __init__(self,name,addr="nullptr" ,size="0"):
        TFLite.__init__(self,name,addr=addr,size=size)

    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "appnodes"

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "KWS"