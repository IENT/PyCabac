import cabac
import numpy as np
import time

# number of bits used for representing the probalities in VTM-cabac
PROB_BITS = 15

# context initialization
p1_init = 0.5
shift_idx = 13

size=100000000

# lets encode a unit step function
bitsToEncode = [0] * size + [1] * size

init_time_normal=time.time()

# create an encoder and encode the bits
enc = cabac.cabacEncoder()
enc.initCtx([(p1_init, shift_idx)])
enc.start()
for i, bit in enumerate(bitsToEncode):
    ctx = 0
    enc.encodeBin(bit, ctx)
enc.encodeBinTrm(1)
enc.finish()
enc.writeByteAlignment()

# get the bitstream 
bs = enc.getBitstream()

# decode in order to sure that we get a enc/dec match
decodedBits = []
dec = cabac.cabacDecoder(bs)
dec.initCtx([(p1_init, shift_idx)])
dec.start()

for i in range(0, len(bitsToEncode)):
    ctx = 0
    decodedBit = dec.decodeBin(ctx)
    decodedBits.append(decodedBit)
dec.decodeBinTrm()
dec.finish()

# assert that the decoded sequence matches the source symbols
assert(bitsToEncode == decodedBits)

print(f"Elapsed time in previous mode->{time.time()-init_time_normal} s")




