from cmsis_stream.cg.scheduler import Graph
from cmsis_stream.cg.scheduler.graphviz import Style

from .nodes import *
from .appnodes import *

from .generate import generate

def generate_camera_test(codeSizeOptimization=False):
    the_graph = Graph()
    
    src = ZephyrDebugVideoSource("video")
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
    
    generate("appc",the_graph,MyStyle(),codeSizeOptimization=codeSizeOptimization)

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(
                    prog='kws',
                    description='Regenerate kws demo')
    parser.add_argument("--size", help="Code size optimization enabled", action='store_true')
    args = parser.parse_args()

    generate_camera_test(codeSizeOptimization=args.size)
    if args.size:
        print("KWS demo generated with code size optimization")
        print("You need to call the generate script to regenerate the common files")
        print("shared between all applications.")