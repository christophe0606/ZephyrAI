from cmsis_stream.cg.scheduler import GenericSink

from .NodeTypes import *

# With selectors, the demo demonstrate how we can extend the event vocabulary.
# Here a new event named "ack" is defined.
# It is used for flow control between this node and the SendToNetwork node.
# Original demo was using the standard "do" event.
class TFLite(GenericSink):
    def __init__(self,name,nbInputs=1,nbOutputs=1,params="nullptr"):
        GenericSink.__init__(self,name,identified=False,selectors=["ack"])
        # Acknowledge event output to tell
        # producer that the network is ready
        self.addEventInput(nbInputs)
        self.addEventOutput(nbOutputs+1)
        self.addVariableArg(params)
        



