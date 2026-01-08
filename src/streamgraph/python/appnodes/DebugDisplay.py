from cmsis_stream.cg.scheduler import GenericSink

from nodes import *

class DebugDisplay(ZephyrLCD):
    def __init__(self,name):
        ZephyrLCD.__init__(self,name)
        self.addEventInput(1)


    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "appnodes"

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "DebugDisplay"