import numpy as np
import struct 

data = np.fromfile("assets/container.bin", dtype=np.uint8)
# hex_bytes = arr.tobytes().hex()

def get_bin_desc(i):
    # Each entry is 4 bytes, so we need to read 4 bytes starting from index i*4
    start = 12 + i*8
    (s,off) = struct.unpack("<II", data[start:start+8])
    return (s,off-0xC0000000)

def get_binary(i):
    (s,off) = get_bin_desc(i)
    return data[off:off+s]

print(get_bin_desc(0))
print(get_bin_desc(1))

print(get_binary(7)[5*12:5*12+16])