from cmsis_stream.cg.scheduler import GenericSink

from nodes import *

class CameraFrame(GenericSink):
    COUNT = 0
    def __init__(self,name):
        GenericSink.__init__(self,name,identified=False)
        CameraFrame.COUNT = CameraFrame.COUNT + 1
        if (CameraFrame.COUNT > 1):
            raise Exception("Only one CameraFrame node can be instantiated")
        self.addEventInput(1)


    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "appnodes"

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "CameraFrame"