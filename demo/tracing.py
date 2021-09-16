import cabac
import numpy as np
import matplotlib.pyplot as plt

# number of bits used for representing the probalities in VTM-cabac
PROB_BITS = 15

# context initialization
p1_init = 0.5
shift_idx = 13

# lets encode a unit step function
bitsToEncode = [0] * 1000 + [1] * 1000

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

# get the bitstream and the encoding trace
bs = enc.getBitstream()
trace = enc.getTrace()

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

# lets check how long our encoded sequence is (in bits)
print(len(bs)*8)

# let's evaluate the trace and get the cabac-internal estimate for p1
p1 = np.array([x[0] for x in trace[0]])
p1 = p1/2**PROB_BITS

# moreover, fetch the value of the mps
mps = np.array([x[1] for x in trace[0]])

# plot both, p1 and mps, over the encoded bins
fig, ax = plt.subplots()
ax.plot(p1)
ax.plot(mps)
ax.plot(bitsToEncode)
ax.set_xlabel("bin")
ax.legend(["p_1", "mps", "source"])
plt.show(block=False)


