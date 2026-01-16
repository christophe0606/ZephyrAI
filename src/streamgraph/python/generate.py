# Generation of CMSIS-Stream schedulers and graph file.s
# It is used by each app in the demo (appa, appb ...) to generate its own scheduler
# It can also be used as a tool to generate a cpp file containing all template instantiations
# used in all the apps of the demo to reduce code size when building all the apps together
import subprocess
from cmsis_stream.cg.scheduler import Configuration
from cmsis_stream.cg.scheduler.graphviz import Style
import argparse 
import json

parser = argparse.ArgumentParser(description='Options for application generation')
# Generate an header file containing extern template declarations to avoid
# instantiation of the templates used in a CMSIS Stream graph
parser.add_argument('--size', action='store_true', help='Generate extern template files')

# When the script is called directly as a tool, the name of all the apps built must be listed
# The script will generate a cpp file containing instantiations of all the templates used in the
# apps.
parser.add_argument('others', nargs=argparse.REMAINDER,help="Apps for which JSON file must be used or generated")


args = parser.parse_args()

# Used to know the name of the json file containing template definitions required by an app
def template_json_file_name(app):
    return f"src/streamgraph/{app}/json/scheduler_{app}_template.json"

def generate(app,the_graph,myStyle):
    conf = Configuration()
    conf.CMSISDSP = False
    conf.asynchronous = False

    conf.horizontal=True
    conf.nodeIdentification = True
    conf.schedName = f"scheduler_{app}"
    conf.schedulerCFileName = f"scheduler_{app}"
    conf.memoryOptimization = True
    # App config is common to all graphs to avoid (by mistake) instantiating the same templates
    # in different graphs and with different implementations (which would be a mistake).
    # If a template depends on some macro definitions, those must be the same for all graphs.
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
    # building several apps together
    conf.prefix = f"stream_{app}_"
    
    # Compute the CMSIS Stream dataflow deterministic schedule (a state machine)
    # and the size of the FIFOs
    scheduling = the_graph.computeSchedule(config=conf)
    
    print("Schedule length = %d" % scheduling.scheduleLength)
    print("Memory usage %d bytes" % scheduling.memory)
    
    # Generate JSON files that may be used by other tools
    scheduling.ccode(f"src/streamgraph/{app}",conf)
    scheduling.genJsonIdentification(f"src/streamgraph/{app}/json",conf)
    scheduling.genJsonSelectors(f"src/streamgraph/{app}/json",conf)
    scheduling.genJsonSelectorsInit(f"src/streamgraph/{app}/json",conf)
    
    def maybeFolder(x):
        if hasattr(x, "folder"):
            return x.folder + "/"
        # Standard nodes from CMSIS Stream package have no folders
        # To implement one-to-many connections in dataflow graph, the scheduling
        # may insert Duplicate nodes. Those nodes come from CMSIS Stream package
        # and thus have no corresponding folder in this demo.
        return ""
    
    # Description of all the classes used in this graph
    # This information is used for:
    # - generating AppNodes_<app>.hpp including all the C++ headers required by this app
    # - generating the extern template header file to avoid multiple instantiation of templates when size mode is on
    # - generating a json file containing the same information for later use when generating the global template instantiation cpp file
    cpp_classes = {}
    
    
    # Get all nodes from this demo and ignore default nodes coming from CMSIS Stream package
    for n in scheduling.allNodes:
        # If it is a node definition coming from this demo:
        if maybeFolder(n):
            # If the node class is not yet listed, add it
            key = f"{n.typeName}{n.ioTemplate()}"
            if key not in cpp_classes:
              isTemplate = True if n.ioTemplate() else False
              cpp_classes[key] = {"folder":maybeFolder(n),"isTemplate":isTemplate,"templateArgs":n.ioTemplate(),"typename":n.typeName,"isIdentified":False}
              # If the node is identified, mark the class as requiring generation of C API
              if n.identified:
                 cpp_classes[key]["isIdentified"] = True
            else:
                # If the class is already tracked, check that any node of same class has identification
                # then it means we need to generate C API for that class
                if n.identified:
                    cpp_classes[key]["isIdentified"] = True

    # identification in the Python description of the graph is per node and not per type.
    # It means : this node has a global identification ID and thus yoi can interact with the node from
    # outside the graph.
    # It means a C API to interact with node of this type has been generated.
    # The C API for the type is thus generated is only one node instance of the type is identified.
                
    # Generate a json file containing the data since it will be used later when generating the global template instantiation cpp file
    with open(template_json_file_name(app),"w") as fc:
        json.dump(cpp_classes,fc,indent=4)
   
    # Generate the AppNodes_<app>.hpp file including all the C++ headers required by this app
    with open(f"src/streamgraph/{app}/AppNodes_{app}.hpp","w") as f:
        for desc in cpp_classes:
            # Generate include for the C++ definition for the node
            # It includes templates and normal C++ classes
            print(f'#include "{cpp_classes[desc]["folder"]}{cpp_classes[desc]["typename"]}.hpp"',file=f)
           
    
    with open(f"src/streamgraph/{app}/{app}.dot","w") as f:
        scheduling.graphviz(f,style=myStyle)
    
    subprocess.run(["dot","-Tpng",f"src/streamgraph/{app}/{app}.dot","-o",f"src/streamgraph/{app}/{app}.png"])

   
    # If size mode is on, we need to generate the extern template header file
    if args.size:       
        # Generate the extern template header file
        with open(ext_template_filename,"w") as f:
            print("// This file is automatically generated. Do not edit.",file=f)
            print(f"#pragma once",file=f)
            for desc in cpp_classes:
                d = cpp_classes[desc]
                type_desc = f"{d['typename']}{d['templateArgs']}"
                if d["isTemplate"]:
                   print(f"extern template class {type_desc};",file=f)
                if d["isIdentified"]:
                   print(f"extern template CStreamNode createStreamNode({type_desc} &obj) ;",file=f)
                   
       


if __name__ == "__main__":
    # When used as a command line tool, it generates a cpp file to instantiate all templates
    # for all the apps (all the graphs) liste on the command line.
    print("Generate template definition file")
   
    cpp_classes = {}    
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
        # and avoid duplicate includes
        # Merge identification requirement for classes already listed
        for app in args.others:
            with open(template_json_file_name(app),"r") as hf:
                hdata = json.load(hf)
                for h in hdata:
                    if h not in cpp_classes:
                        cpp_classes[h] = hdata[h]
                    else:
                        # If the class is already listed, check if it requires C API generation
                        if hdata[h]["isIdentified"]:
                            cpp_classes[h]["isIdentified"] = True

       
        # Generate all includes to have template definitions
        # We don't list normal C++ classes here. Only templates
        print("",file=f)
        for h in cpp_classes:
            headerNeeded = cpp_classes[h]["isTemplate"] or cpp_classes[h]["isIdentified"]
            if headerNeeded:
               print(f'#include "{cpp_classes[h]["folder"]}{cpp_classes[h]["typename"]}.hpp"',file=f)
            
        print("",file=f)
        # Generate all template instantiations
        for h in cpp_classes:
            d = cpp_classes[h]
            type_desc = f"{d['typename']}{d['templateArgs']}"
            if d["isTemplate"]:
                print(f"template class {type_desc};",file=f)
            if d["isIdentified"]:
                print(f"template CStreamNode createStreamNode({type_desc} &obj) ;",file=f)
