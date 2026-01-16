from cmsis_stream.cg.scheduler import GenericSink

from .NodeTypes import *

class TFLite(GenericSink):
    def __init__(self,name,nbInputs=1,nbOutputs=1,params="nullptr"):
        GenericSink.__init__(self,name,identified=False)
        # Acknowledge event output to tell
        # producer that the network is ready
        self.addEventInput(nbInputs)
        self.addEventOutput(nbOutputs+1)
        self.addVariableArg(params)
        



