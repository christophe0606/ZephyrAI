from cmsis_stream.cg.scheduler import GenericSource

class DebugSource(GenericSource):
    def __init__(self,name,theType,outLength,frequency=440,samplingFreq=16000,master=False):
        GenericSource.__init__(self,name,identified=False)
        # Stereo output
        self.addOutput("o",theType,outLength)
        self.addLiteralArg(frequency)
        self.addLiteralArg(samplingFreq)
        self.addLiteralArg(1 if master else 0)

    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "appnodes"

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "DebugSource"