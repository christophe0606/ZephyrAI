# Generation of CMSIS-Stream scheduler and graph file.
# This is common to all examples
import subprocess
from cmsis_stream.cg.scheduler import Configuration
from cmsis_stream.cg.scheduler.graphviz import Style
import argparse 
import json

parser = argparse.ArgumentParser(description='Options for application generation')
# Generate an header file containing extern template declarations to avoid
# instantiation of the template for the graph
parser.add_argument('--size', action='store_true', help='Generate extern template files')

# When the script is called directly as a tool, the name of all the apps built must be listed
# The script will generate a cpp file containing instantiations of all the templates used in the
# apps.
parser.add_argument('others', nargs=argparse.REMAINDER,help="Apps for which JSON file must be used or generated")


args = parser.parse_args()

# Used to know the name of the json file containing the selectors init info
# It also describe which nodes are templates and gives the template parameters
def json_file_name(app):
    return f"src/streamgraph/{app}/json/scheduler_{app}_selectors_inits.json"

def generate(app,the_graph,myStyle):
    conf = Configuration()
    conf.CMSISDSP = False
    conf.asynchronous = False

    conf.horizontal=True
    conf.nodeIdentification = True
    conf.schedName = f"scheduler_{app}"
    conf.schedulerCFileName = f"scheduler_{app}"
    conf.memoryOptimization = True
    # App config is common to all graphs to avoid (by mistake) instantiate the same templates
    # in different graphs and with different implementations (which would be a mistake).
    # If template depend son some macro definitions, those must be the same for all graphs.
    # This app config should be used only to inject code before schedule or node execution.
    # It should not customize node implementations.
    conf.appConfigCName = f"app_config.hpp"
    conf.cOptionalInitArgs = [f"{app.capitalize()}Params *params"]
    conf.appNodesCName = f"AppNodes_{app}.hpp"
    ext_template_filename = f"src/streamgraph/{app}/{app}_extern_templates.hpp"
    # Post custom headers to include external template declarations
    if args.size:
        conf.postCustomCName = f"{app}_extern_templates.hpp"

    # Alif code is defining some variables as buf0, buf1 and not static
    # They conflict with the buf0, buf1 defined by stream
    # So a prefix is added and it contains the app name to avoid conflicts when
    # building several apps
    conf.prefix = f"stream_{app}_"
    
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
    
    with open(f"src/streamgraph/{app}/AppNodes_{app}.hpp","w") as f:
        # Generate AppNodes header to include all definitions
        # and json file used for template instantiation later
        s = set([(maybeFolder(x),x.typeName,x) for x in scheduling.allNodes])
        headers = set()
        for folder,n,node in s:
            # Some default nodes coming from CMSIS Stream may be used
            # They have no folder and they don't need to be included through AppNodes 
            # since they are already included by standard headers
            if folder:
               print(f'#include "{folder}{n}.hpp"',file=f)
               # If the node is a template, we need to include its header in the json file
               headers.add(f"{folder}{n}.hpp")
              

        # Generate a json file containing the path to all template headers that are
        # used in the graph. It is not like AppNodes because it contains only templates
        # Those headers are used when templates are instantiated in a separate C++ file
        # since we need to include their definitions
        with open(f"src/streamgraph/{app}/json/{app}_headers.json","w") as fc:
            h = list(headers)
            json.dump(h,fc,indent=4)
        
               
    
    with open(f"src/streamgraph/{app}/{app}.dot","w") as f:
        scheduling.graphviz(f,style=myStyle)
    
    subprocess.run(["dot","-Tpng",f"src/streamgraph/{app}/{app}.dot","-o",f"src/streamgraph/{app}/{app}.png"])

    # If size optimization is requested, we generate a new header that list
    # all template used in the graph and declare them as extern template
    # This header is added to the scheduler as a post custom header file
    if args.size:
        data_class = {} 
        data_ident = {}
        with open(json_file_name(app), "r") as jf:
             # Load the json data containing the template information we need
             data_class = json.load(jf)
        with open(ext_template_filename,"w") as f:
            print("// This file is automatically generated. Do not edit.",file=f)
            print(f"#ifndef EXTERN_TEMPLATES_{app.upper()}_HPP",file=f)
            print(f"#define EXTERN_TEMPLATES_{app.upper()}_HPP\n",file=f)
            for node in data_class:
                desc = data_class[node]
                if desc["isTemplate"]:
                   print(f"extern template class {node};",file=f)
                print(f"extern template CStreamNode createStreamNode({node} &obj) ;",file=f)

           
                   
            print(f"#endif // EXTERN_TEMPLATES_{app.upper()}_HPP",file=f)


if __name__ == "__main__":
    # When used as a command line tool, it generates a cpp file to instantiate all templates
    # for all the apps (all the graphs) liste on the command line.
    print("Generate template definition file")
    # Template to instantiate
    templates = {}
    # Header definition for those templates and other classes
    headers = set()
    
    with open("src/streamgraph/common/template_instantiations.cpp","w") as f:
        print("// This file is automatically generated. Do not edit.\n",file=f)
        print(f'#include <cstdint>',file=f)
        print(f'#include "app_config.hpp"',file=f)
        print(f'#include "stream_platform_config.hpp"',file=f)
        print(f'#include "cg_enums.h"',file=f)
        print(f'#include "StreamNode.hpp"',file=f)
        print(f'#include "cstream_node.h"',file=f)
        print(f'#include "IdentifiedNode.hpp"',file=f)
        print(f'#include "EventQueue.hpp"',file=f)
        print(f'#include "GenericNodes.hpp"\n',file=f)

        # Read all JSON file to get the list of headers to include
        for app in args.others:
            with open(f"src/streamgraph/{app}/json/{app}_headers.json","r") as hf:
                hdata = json.load(hf)
                for h in hdata:
                    headers.add(h)

        # Generate all includes to have template definitions
        print("",file=f)
        for h in headers:
            print(f'#include "{h}"',file=f)
            
        # Read other json files to get the list of templates to instantiate
        print("",file=f)
        for app in args.others:
            with open(json_file_name(app), "r") as jf:
                 data = json.load(jf)
                 templates = templates | data
        for template in templates:
            desc = templates[template]
            if desc["isTemplate"]:
                print(f"template class {template};",file=f)
            print(f"template CStreamNode createStreamNode({template} &obj) ;",file=f)

