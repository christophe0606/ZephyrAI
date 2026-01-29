import argparse
import re 
import struct
import hashlib
import test

parser = argparse.ArgumentParser(description='Convert C network to binary')
parser.add_argument('-f', nargs='?',type = str, default="networks.bin", help="Binary file")
parser.add_argument('-x', nargs='?',type = int, default=0xC0000000, help="Ext mem start address")
parser.add_argument('-d', action='store_true', help="Debug")

parser.add_argument('c', nargs=1,help="C network file")

args = parser.parse_args()

ALIGN=16
BEFORE=1
INNETWORK=2


# Get a network either from a binary file or from the .cpp file containing the C array
# return a byte stream with the network
def get_network(filename):
    if re.search(r'\.bin$',filename):
        with open(filename,"rb") as f:
            return f.read()
    state = 1 
    network = bytes()
    
    with open(filename,"r") as f:
        for line in f:
            if state == BEFORE:
                if re.match(r'^static const uint8_t nn_model.*$',line):
                    state = INNETWORK
            elif state == INNETWORK:
                line_bytes = [int(x,16) for x in re.findall(r'0x[0-9a-fA-F]{2}',line)]
                if line_bytes:
                   data = struct.pack(f'<{len(line_bytes)}B', *line_bytes)
                   network += data
                if re.search(r'};',line):
                     break
   
    return network
            
# Get all networks passed as command line arguments
networks = [get_network(x) for x in args.c]

if args.d:
    print("Debug mode. Only first network written. No header written.")
    with open(args.f,"wb") as f:
        f.write(networks[0])
        print(f"Length of network 0x{len(networks[0]):08X}")
else:
    # Write network description header and all the networks
    # networks are aligned in memory
    with open(args.f,"wb") as f:
        offset = 4 # initial offset for length of full description
        # But length written at the end. At start only 4 bytes are reserved for it.
        network_desc = bytes() # Description of networks in memory
        header = bytes() # 
        # Add magic number for debug
        header += struct.pack('<II',0xBEEFDEAD,len(networks))
        network_desc += header
        offset += len(header)
        offset += len(networks)*8  # space for table of network description to be filled below
    
        the_networks = bytes() # Network binaries to add to description
        # Network lengths and pos_and_network
        pos_and_network = []
        for net in networks:
            # align offset
            if offset % ALIGN != 0:
                pad = ALIGN - (offset % ALIGN)
                # Add padding to align network
                the_networks += bytes([0]*pad)
                offset += pad
            
            # Track network length and position
            pos_and_network.append((len(net),offset + args.x))
            # Add network binary 
            the_networks += net 
            offset += len(net)
            
        # Add network descriptions to header
        for len,off in pos_and_network:
            network_desc += struct.pack('<II',len,off)
    
        # Finally add all networks
        network_desc += the_networks
        # Update length at start of description
        network_desc = struct.pack('<I',offset) + network_desc
        f.write(network_desc)
        print(f"Length of description {offset:08X}")
    
        # Compute hash of network description
        md5 = hashlib.md5(network_desc).hexdigest()
        print(f"md5 hash = {md5}")



