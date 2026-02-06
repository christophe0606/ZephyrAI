import argparse
import re 
import struct
import hashlib
import test

parser = argparse.ArgumentParser(description='Package C networks and binaries into binary container')
parser.add_argument('-f', nargs='?',type = str, default="assets/container.bin", help="Binary container file")
parser.add_argument('-x', nargs='?',type = int, default=0xC0000000, help="Ext mem start address")
parser.add_argument('-d', action='store_true', help="Debug")

parser.add_argument('c', nargs='+',help="C network files and binary files to package")

args = parser.parse_args()

ALIGN=16
BEFORE=1
INNETWORK=2


# Get a binary either from a binary file or from the .cpp file containing the binary C array
# return a byte stream with the binary content
def get_binary(filename):
    if re.search(r'\.bin$',filename):
        with open(filename,"rb") as f:
            return f.read()
    state = 1 
    binary = bytes()
    
    with open(filename,"r") as f:
        for line in f:
            if state == BEFORE:
                if re.match(r'^static const uint8_t nn_model.*$',line):
                    state = INNETWORK
            elif state == INNETWORK:
                line_bytes = [int(x,16) for x in re.findall(r'0x[0-9a-fA-F]{2}',line)]
                if line_bytes:
                   data = struct.pack(f'<{len(line_bytes)}B', *line_bytes)
                   binary += data
                if re.search(r'};',line):
                     break
   
    return binary
            
# Get all binaries passed as command line arguments
all_binaries = [get_binary(x) for x in args.c]

if args.d:
    print("Debug mode. Only first binary written. No header written.")
    with open(args.f,"wb") as f:
        f.write(all_binaries[0])
        print(f"Length of binary 0x{len(all_binaries[0]):08X}")
else:
    # Write binary description header and all the binaries
    # binaries are aligned in memory
    with open(args.f,"wb") as f:
        offset = 4 # initial offset for length of full description
        # But length written at the end. At start only 4 bytes are reserved for it.
        binary_desc = bytes() # Description of binaries in memory
        header = bytes() # 
        # Add magic number for debug
        header += struct.pack('<II',0xBEEFDEAD,len(all_binaries))
        binary_desc += header
        offset += len(header)
        offset += len(all_binaries)*8  # space for table of binary description to be filled below
    
        the_binaries = bytes() # Network binaries to add to description
        # Network lengths and pos_and_binary
        pos_and_binary = []
        for net in all_binaries:
            # align offset
            if offset % ALIGN != 0:
                pad = ALIGN - (offset % ALIGN)
                # Add padding to align binary
                the_binaries += bytes([0]*pad)
                offset += pad
            
            # Track binary length and position
            pos_and_binary.append((len(net),offset + args.x))
            # Add binary binary 
            the_binaries += net 
            offset += len(net)
            
        # Add binary descriptions to header
        for len,off in pos_and_binary:
            binary_desc += struct.pack('<II',len,off)
    
        # Finally add all binaries
        binary_desc += the_binaries
        # Update length at start of description
        binary_desc = struct.pack('<I',offset) + binary_desc
        f.write(binary_desc)
        print(f"Length of description {offset:08X}")
    
        # Compute hash of binary description
        md5 = hashlib.md5(binary_desc).hexdigest()
        print(f"md5 hash = {md5}")



