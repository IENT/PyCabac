import numpy as np
import math

def encode_bi(symbol, width):
    symbol = np.asarray(symbol)
    if symbol.size == 1:  # scalar
        return np.array( # convert to binary string and then to array of bits
            list(np.binary_repr(symbol).zfill(width)),
            dtype=np.uint8)
    
        binary_string = format(int(x), "0"  + str(n) + "b")
        return np.array([
            int(x) for x in binary_string
        ])
    else:  # vectorized
        return (np.bitwise_and( # bitwise and with powers of 2
            symbol, 1 << np.arange(width)[::-1, None]
        ) > 0 ).astype(np.uint8)


def decode_bi(bits):
    return np.dot( # dot product with powers of 2
        1 << np.arange(bits.shape[0])[::-1],
        bits
    )

    result = 0
    for i in range(len(x)):
        if x[i]:
            result += math.pow(2, len(x)-i-1)
    return int(result)

def encode_tu(x, xmax=512):

    bits = np.repeat(1, repeats=x+1)  # create array of ones with length x+1
    if x < xmax:
        bits[-1] = 0  # replace 1 with 0 (we have x ones and one terminating zero)
    else:
        bits = bits[:-1]  # delete last element for xmax (we have xmax ones and no terminating zero)

    return bits


def decode_tu(bits):
    
    x = np.sum(bits)
    return x


# Encode integer symbol to Exponential-Golomb code with order k
def encode_eg(x, k, return_prefix_suffix=False):

    if x == 0 and k == 0:
        return [1]

    p = math.floor(math.log2(abs(x) + (1 << k))) - k
    m = (1 << (k + p)) - (1 << k)

    prefix = encode_bi(1,   p+1)
    suffix = encode_bi(x-m, k+p)
    code =  np.concatenate((
        prefix,
        suffix
    ))

    if return_prefix_suffix:
        return code, prefix, suffix
    else:
        return code

