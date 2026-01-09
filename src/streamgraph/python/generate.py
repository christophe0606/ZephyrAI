# Generation of CMSIS-Stream scheduler and graph file.
# This is common to all examples
import subprocess
from cmsis_stream.cg.scheduler import Configuration
from cmsis_stream.cg.scheduler.graphviz import Style

def generate(app,graph_name,the_graph,myStyle):
    conf = Configuration()
    conf.CMSISDSP = False
    conf.asynchronous = False
    conf.horizontal=True
    conf.nodeIdentification = True
    conf.schedName = f"scheduler_{graph_name}"
    conf.schedulerCFileName = f"scheduler_{graph_name}"
    conf.memoryOptimization = True
    conf.appConfigCName = f"{graph_name}_custom_config.hpp"
    conf.cOptionalInitArgs = ["GraphaParams *params"]
    conf.appNodesCName = f"AppNodes_{graph_name}.hpp"

    # Alif code is defining some variables as buf0, buf1 and not static
    # They conflict with the buf0, buf1 defined by stream
    # So a prefix is added 
    conf.prefix = "stream"
    
    scheduling = the_graph.computeSchedule(config=conf)
    
    print("Schedule length = %d" % scheduling.scheduleLength)
    print("Memory usage %d bytes" % scheduling.memory)
    
    scheduling.ccode(f"src/streamgraph/{app}",conf)
    scheduling.genJsonIdentification(f"src/streamgraph/{app}/json",conf)
    scheduling.genJsonSelectors(f"src/streamgraph/{app}/json",conf)
    scheduling.genJsonSelectorsInit(f"src/streamgraph/{app}/json",conf)
    
    def maybeFolder(x):
        if hasattr(x, "folder"):
            return x.folder + "/"
        # Standard ndoes from cmsis stream package have no folders
        return ""
    
    with open(f"src/streamgraph/{app}/AppNodes_{graph_name}.hpp","w") as f:
        #print(scheduling.allNodes)
        s = set([(maybeFolder(x),x.typeName) for x in scheduling.allNodes])
        for folder,n in s:
            if folder:
               print(f'#include "{folder}{n}.hpp"',file=f)
               
    
    with open(f"src/streamgraph/{app}/{graph_name}.dot","w") as f:
        scheduling.graphviz(f,style=myStyle)
    
    subprocess.run(["dot","-Tpng",f"src/streamgraph/{app}/{graph_name}.dot","-o",f"src/streamgraph/{app}/{graph_name}.png"])