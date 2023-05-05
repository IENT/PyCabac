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
#bitsToEncode = [0] * size + [1] * size
#bitsToEncode=np.array(bitsToEncode,dtype=np.uint8)


bitsToEncode=np.concatenate((np.zeros(size,dtype=np.uint8),np.ones(size,dtype=np.uint8)))

init_time_numpy=time.time()

# create an encoder and encode the bits
enc = cabac.cabacEncoder()
enc.initCtx([(p1_init, shift_idx)])
enc.start()
enc.encodeBinsWrapperNumpy(bitsToEncode)
enc.encodeBinTrm(1)
enc.finish()
enc.writeByteAlignment()

# get the bitstream 
bs = np.array(enc.getBitstreamWrapperNumpy(),dtype=np.uint8,copy=False)

# decode in order to sure that we get a enc/dec match
dec = cabac.cabacDecoder(bs)
dec.initCtx([(p1_init, shift_idx)])
dec.start()
decodedBits=np.array(dec.decodeBinsWrapperNumpy(len(bitsToEncode)),dtype=np.uint8,copy=False)
dec.finish()

# assert that the decoded sequence matches the source symbols
assert((bitsToEncode == decodedBits).all())#np.array_equal(bitsToEncode,decodedBits)

print(f"Elapsed time in numpy no-copy mode->{time.time()-init_time_numpy} s")




