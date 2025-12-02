from cmsis_stream.cg.scheduler import GenericSink

from .NodeTypes import *

class ZephyrLCD(GenericSink):
    def __init__(self,name):
        GenericSink.__init__(self,name)

