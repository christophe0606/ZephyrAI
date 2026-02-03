from cmsis_stream.cg.scheduler import GenericSink
    
# With selectors, the demo demonstrate how we can extend the event vocabulary.
# Here a new event named "ack" is defined.
# It is used for flow control between this node and the TFLite node.
# Original demo was using the standard "do" event.
class SendToNetwork(GenericSink):
    def __init__(self,name,theType,nbSamples):
        GenericSink.__init__(self,name,identified=True,selectors=["ack"])
        self.addInput("i",theType,nbSamples)
        self.addEventInput()
        self.addEventOutput()

    @property
    def folder(self):
        """The folder where the C++ implementation of this node is located"""
        return "nodes"
    
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "SendToNetwork"