from cmsis_stream.cg.scheduler import Graph
from cmsis_stream.cg.scheduler.graphviz import Style

from nodes import *
from appnodes import *

from generate import generate


the_graph = Graph()

src = ZephyrVideoSource("video")
gain = CameraFrame("lcd")

the_graph.connect(src["oev0"],gain["iev0"])

class MyStyle(Style):
    
    def edge_color(self,edge):
        nb = self.fifoLength(edge) 
        if nb is None:
            nb = 0
        s = self.edgeSrcNode(edge)
        d = self.edgeDstNode(edge)
        
        if d.nodeName ==  "display":
           return("magenta")
        else: 
            if (nb > 512):
                return("orange")
            return(super().edge_color(edge))

generate(the_graph,Style())
