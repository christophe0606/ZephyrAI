from cmsis_stream.cg.scheduler import GenericSink

from ..nodes import ZephyrLCD

class SpectrogramDisplay(ZephyrLCD):
    def __init__(self,name):
        ZephyrLCD.__init__(self,name)
        self.addEventInput(2)


    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "appnodes"

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "SpectrogramDisplay"