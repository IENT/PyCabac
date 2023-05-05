import unittest
import random
import cabac
import math


class MainTest(unittest.TestCase):

    def test_ctx_selection(self):

        def ctx_id_tu(n, prev_symbol=0, n_rst=10):
            # n is the position of the to-be coded bin in the bin string (of the to be coded symbol)
            # prev_symbol is the previously coded symbol
            # bins with n < n_rst are coded with ctx model, all other bins are coded with the same context.
            # returns the context id
            #
            # First ctx_ids correspond to previous bin at position n not available case.
            # Next ctx_ids correspond to previous bin at position n = 0 case.
            # Next ctx_ids correspond to previous bin at position n = 1 case.
            # Last ctx_id is used for all bins n >= n_rst

            import numpy as np  # TODO: move this import somewhere else

            n = np.asarray(n)
            ctx_id = np.zeros(n.shape, dtype=np.int32)

            # binary masks
            mask_rst = n < n_rst  # mask for rest case
            mask_prev_symbol_na = mask_rst & (n > prev_symbol)  # mask for previous bin not available
            mask_prev_symbol_v0 = mask_rst & (n < prev_symbol)  # mask for previous bin 0
            mask_prev_symbol_v1 = mask_rst & (n == prev_symbol) # mask for previous bin 1

            # bin position n < n_rst
            # previously coded bin at same position not available
            ctx_id[mask_prev_symbol_na] = 0*n_rst + n[mask_prev_symbol_na]

            # previously coded bin 0 at same position n
            ctx_id[mask_prev_symbol_v0] = 1*n_rst + n[mask_prev_symbol_v0]

            # previously coded bin 1 at same position n
            ctx_id[mask_prev_symbol_v1] = 2*n_rst + n[mask_prev_symbol_v1]

            # bin position n >= n_rst
            # rst case, independent of position n
            ctx_id[~mask_rst] = 3*n_rst

            return ctx_id
        

    def test_tu(self):
        num_max_val = 255
        num_values = 1000
        symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        

        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(1, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()
        for symbol in symbols:
            enc.encodeBinsTUbypass(symbol)

        ctx_ids = [0] * (num_max_val+2)
        for symbol in symbols:
            enc.encodeBinsTU(symbol, ctx_ids, num_max_val)
            #enc.encodeBinsBI(symbol, ctx_ids, 8)
            #enc.encodeBinsBIbypass(symbol, 8)
        
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        dec = cabac.cabacSimpleSequenceDecoder(bs)
        dec.start()

        decodedSymbols = []
        for _ in range(0, num_values):
            decodedSymbol = dec.decodeBinsTUbypass()
            decodedSymbols.append(decodedSymbol)

        ctx_ids = [0] * (num_max_val+2)
        for _ in range(0, num_values):
            decodedSymbol = dec.decodeBinsTU(ctx_ids)
            #decodedSymbol = dec.decodeBinsBI(ctx_ids, 8)
            #decodedSymbol = dec.decodeBinsBIbypass(8)
            decodedSymbols.append(decodedSymbol)

        
        dec.decodeBinTrm()
        dec.finish()

        self.assertTrue(decodedSymbols == symbols)



    def test_ep(self):
        symbol = random.randint(0, 511)
        Nbits = math.ceil(math.log2(symbol))

        enc = cabac.cabacEncoder()
        enc.start()
        enc.encodeBinsEP(symbol, Nbits)
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        decodedBits = []
        dec = cabac.cabacDecoder(bs)


        dec.start()
        decodedSymbol = dec.decodeBinsEP(Nbits)

        dec.decodeBinTrm()
        dec.finish()

        self.assertTrue(decodedSymbol == symbol)


    def test_enc_dec(self):

        p1_init = 0.6
        shift_idx = 8
        bitsToEncode = [random.randint(0, 1) for _ in range(0, 1000)]
        enc = cabac.cabacEncoder()
        enc.initCtx([(p1_init, shift_idx), (p1_init, shift_idx)])
        enc.start()
        for i, bit in enumerate(bitsToEncode):
            if i != 0:
                ctx = bitsToEncode[i - 1]
            else:
                ctx = 0
            enc.encodeBin(bit, ctx)
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        decodedBits = []
        dec = cabac.cabacDecoder(bs)
        dec.initCtx([(p1_init, shift_idx), (p1_init, shift_idx)])
        dec.start()

        for i in range(0, len(bitsToEncode)):
            if i != 0:
                ctx = decodedBits[i - 1]
            else:
                ctx = 0
            decodedBit = dec.decodeBin(ctx)
            decodedBits.append(decodedBit)
        dec.decodeBinTrm()
        dec.finish()

        self.assertTrue(decodedBits == bitsToEncode)

    def test_context_init(self):

        # lets encode a unit step function with the smallest possible shift_idx
        # IMHO this model should adapt quite quickly and result in the lowest
        # possible bitrate
        p1_init = 0
        shift_idx = 0
        bitsToEncode = [0] * 1000 + [1] * 1000
        enc = cabac.cabacEncoder()
        enc.initCtx([(p1_init, shift_idx)])
        enc.start()
        for i, bit in enumerate(bitsToEncode):
            ctx = 0
            enc.encodeBin(bit, ctx)
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

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

        len1 = len(bs)

        # Try with a larger shift_idx. This should end a a higher bitrate
        shift_idx = 8
        enc = cabac.cabacEncoder()
        enc.initCtx([(p1_init, shift_idx)])
        enc.start()
        for i, bit in enumerate(bitsToEncode):
            ctx = 0
            enc.encodeBin(bit, ctx)
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

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

        len2 = len(bs)

        self.assertTrue(len1 < len2)


if __name__ == "__main__":
    unittest.main()
