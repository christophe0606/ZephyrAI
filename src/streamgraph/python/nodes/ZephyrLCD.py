from cmsis_stream.cg.scheduler import GenericSink

from .NodeTypes import *

class ZephyrLCD(GenericSink):
    COUNT = 0
    def __init__(self,name):
        GenericSink.__init__(self,name)
        ZephyrLCD.COUNT = ZephyrLCD.COUNT + 1
        if (ZephyrLCD.COUNT > 1):
            raise Exception("Only one ZephyrLCD node can be instantiated")
        

