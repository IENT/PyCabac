import matplotlib.pyplot as plt

import cabac

# This demo shows how to use the tracing functionality of the CABAC module.
# More specifically, we will use the "cabacTraceEncoder" class.
# Tracing allows to get the internal state of the CABAC encoder at each bin
# for each context.
# This can be used to evaluate the internal estimates for the probability of the next bin (p1)
# and the most probable symbol (mps).

# Let's encode a unit step function
bitsToEncode = [0] * 1000 + [1] * 1000

# Setup CABAC
p1_init = 0.5  # Initial value for p1
shift_idx = 13   # change to e.g. 0 to see the effect of faster adaptation

# Number of bits used for representing the probabilities in VTM-CABAC
PROB_BITS = 15  # Don't change this

# Create an encoder
enc = cabac.cabacTraceEncoder()
enc.initCtx([(p1_init, shift_idx)])
enc.start()

# Encode the bits
for i, bit in enumerate(bitsToEncode):
    ctx = 0
    enc.encodeBin(bit, ctx)
enc.encodeBinTrm(1)
enc.finish()
enc.writeByteAlignment()

# Get the bitstream and the encoding trace
bs = enc.getBitstream()
trace = enc.getTrace()

# Decode in order to ensure that we get a enc/dec match
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

# Assert that the decoded sequence matches the source symbols
assert(bitsToEncode == decodedBits)

# Evaluation
# Let's check how long our encoded sequence is (in bits)
print(len(bs)*8)

# Let's evaluate the trace and get the cabac-internal estimate for p1
p1 = [x[0]/2**PROB_BITS for x in trace[0]]
p1 = p1

# Moreover, fetch the value of the mps
mps = [x[1] for x in trace[0]]

# Plot both, p1 and mps, over the encoded bins
fig, ax = plt.subplots()
ax.plot(p1)
ax.plot(mps)
ax.plot(bitsToEncode)
ax.set_xlabel("bin")
ax.legend(["p_1", "mps", "source"])
plt.show()


