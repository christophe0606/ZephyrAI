from cmsis_stream.cg.scheduler import GenericSink

from ..nodes import TFLite

class KWS(TFLite):
    def __init__(self,name):
        TFLite.__init__(self,name,
                        params=f"params->{name}")

    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "appnodes"

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "KWS"