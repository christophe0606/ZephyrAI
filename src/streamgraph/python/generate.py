# Generation of CMSIS-Stream scheduler and graph file.
# This is common to all examples
import subprocess
from cmsis_stream.cg.scheduler import Configuration
from cmsis_stream.cg.scheduler.graphviz import Style

def generate(graph_name,the_graph,myStyle):
    conf = Configuration()
    conf.CMSISDSP = False
    conf.asynchronous = False
    conf.horizontal=True
    conf.nodeIdentification = True
    conf.schedName = f"scheduler_{graph_name}"
    conf.schedulerCFileName = f"scheduler_{graph_name}"
    conf.memoryOptimization = True
    conf.appConfigCName = f"{graph_name}_custom_config.hpp"
    conf.cOptionalArgs = "void *graphData"

    # Alif code is defining some variables as buf0, buf1 and not static
    # They conflict with the buf0, buf1 defined by stream
    # So a prefix is added 
    conf.prefix = "stream"
    
    scheduling = the_graph.computeSchedule(config=conf)
    
    print("Schedule length = %d" % scheduling.scheduleLength)
    print("Memory usage %d bytes" % scheduling.memory)
    
    scheduling.ccode("src/streamgraph/scheduler",conf)
    scheduling.genJsonIdentification("src/streamgraph/json",conf)
    scheduling.genJsonSelectors("src/streamgraph/json",conf)
    scheduling.genJsonSelectorsInit("src/streamgraph/json",conf)
    
    def maybeFolder(x):
        if hasattr(x, "folder"):
            return x.folder + "/"
        # Standard ndoes from cmsis stream package have no folders
        return ""
    
    with open("src/streamgraph/scheduler/AppNodes.hpp","w") as f:
        #print(scheduling.allNodes)
        s = set([(maybeFolder(x),x.typeName) for x in scheduling.allNodes])
        for folder,n in s:
            if folder:
               print(f'#include "{folder}{n}.hpp"',file=f)
               
    
    with open("src/streamgraph/scheduler/graph.dot","w") as f:
        scheduling.graphviz(f,style=myStyle)
    
    subprocess.run(["dot","-Tpng","src/streamgraph/scheduler/graph.dot","-o","src/streamgraph/scheduler/graph.png"])