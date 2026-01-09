from cmsis_stream.cg.scheduler import GenericSink

from nodes import *

class KWSClassify(GenericSink):
    def __init__(self,name):
        GenericSink.__init__(self,name)
        self.addEventInput(1)
        self.addEventOutput(1)
        self.addVariableArg("evtQueue")
        self.addLiteralArg(8)

    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "appnodes"

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "KWSClassify"