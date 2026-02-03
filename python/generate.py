# Generation of CMSIS-Stream schedulers and graph file.s
# It is used by each app in the demo (appa, appb ...) to generate its own scheduler
# It can also be used as a tool to generate a cpp file containing all template instantiations
# used in all the apps of the demo to reduce code size when building all the apps together
import subprocess
from cmsis_stream.cg.scheduler import Configuration
from cmsis_stream.cg.scheduler.graphviz import Style
import argparse 
import json



# Used to know the name of the json file containing template definitions required by an app
def template_json_file_name(app):
    return f"src/streamgraph/{app}/json/scheduler_{app}_template.json"

# Used by network generation scripts to generate the scheduler and related files for an app
def generate(app,the_graph,myStyle,codeSizeOptimization=False):
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
    if codeSizeOptimization:
        conf.postCustomCName = f"{app}_extern_templates.hpp"

    # A global selector initialization file is generated
    conf.disableSelectorInit = True

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
    # It means : this node has a global identification ID and thus you can interact with the node from
    # outside the graph.
    # It means a C API to interact with node of this type has been generated.
    # The C API for the type is thus generated if only one node instance of the type is identified.
                
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
    if codeSizeOptimization:       
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
                   
# Generate common template and selector files used by all apps
def generate_common_files(all_apps=[]): 
    #
    # Read all JSON file to get the list of headers to include
    # and avoid duplicate includes
    # Merge identification requirement for classes already listed
    cpp_classes = {}    
    node_with_selectors = {}
    selectors = set()
    # Read all template json files generated for each app
    # Each template json file describes the types  used for each node in the graph
    # We use this data to know if a type is a template, if there is one node 
    # instance of this type that is identified (thus requiring C API generation)
    # and the folder where to find the C++ header defining the type
    # Later we will merge selector information to know which types require selector initialization
    for app in all_apps:
        with open(template_json_file_name(app),"r") as hf:
            hdata = json.load(hf)
            for h in hdata:
                if h not in cpp_classes:
                    cpp_classes[h] = hdata[h]
                else:
                    # If the class is already listed, check if it requires C API generation
                    if hdata[h]["isIdentified"]:
                        cpp_classes[h]["isIdentified"] = True
        # Read the selector data from the graph and merge some info inside cpp_classes
        # to know later if the CPP class requires selector initialization
        # It is used to know which headers to include in the global template instantiation cpp file
        # And we also track all selectors to generate unique IDs for them
        with open(f"src/streamgraph/{app}/json/scheduler_{app}_selectors_inits.json","r") as sf:
            sdata = json.load(sf)
            for sel in sdata:
                node_desc = sdata[sel]
                if "selectors" in node_desc:
                    if not sel in node_with_selectors and node_desc["selectors"]:
                       node_with_selectors[sel] = node_desc
                       if sel in cpp_classes:
                           cpp_classes[sel]["hasSelectors"] = True
                    for v in node_desc["selectors"]:
                        selectors.add(v)

    with open("src/streamgraph/common/selector_ids.h","w") as f:
        print(f'''#ifndef SELECTOR_IDS_H
#define SELECTOR_IDS_H
// This file is automatically generated. Do not edit.
              
// Selector IDs
#ifdef   __cplusplus
extern "C"
{{
#endif

''',file=f)
        
        # Generate unique identifier for the selector starting at 100
        id = 100
        for s in selectors:
            print(f"#define {s} {id}",file=f)
            id = id + 1


        print(f'''
              
#ifdef   __cplusplus
}}
#endif
#endif // SELECTOR_IDS_H
''',file=f)
       
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

        
        # Generate all includes to have template definitions
        # We don't list normal C++ classes here. Only templates
        print("",file=f)
        for h in cpp_classes:
            headerNeeded = cpp_classes[h]["isTemplate"] or cpp_classes[h]["isIdentified"] or cpp_classes[h]["hasSelectors"]
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

        
        # Generate initialization of all selectors
        print("\n// Selector initializations",file=f)
        
        
        for n in node_with_selectors:
            node = node_with_selectors[n]
            selString = ", ".join([str(x) for x in node["selectors"]])
            if node["isTemplate"]:
               print(f'template<>\nstd::array<uint16_t,{len(node["selectors"])}> {n}::selectors = {{{selString}}};',file=f)
            else:
               print(f'std::array<uint16_t,{len(node["selectors"])}> {n}::selectors = {{{selString}}};',file=f)
  

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
                    prog='generate',
                    description='Regenerate common files for some apps')

    parser.add_argument('apps', nargs="*",choices=["appa","appb","appc"], help='Applications to regenerate')


    args = parser.parse_args()
    print("Generate template definition files")
    print(args.apps)
    generate_common_files(args.apps)