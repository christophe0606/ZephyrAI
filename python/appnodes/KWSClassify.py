from cmsis_stream.cg.scheduler import GenericSink


class KWSClassify(GenericSink):
    def __init__(self,name):
        GenericSink.__init__(self,name,identified=True)
        self.addEventInput(1)
        self.addEventOutput(1)
        # The Python script somewhere should generate
        # a C file initializing the params
        # structure with initial settings.
        # The settings would come from the instantiation of the nodes
        # in the Python.
        self.addVariableArg(f"params->{name}")

    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "appnodes"

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "KWSClassify"