import unittest
import random
import cabac
import math

import tests.utils.symbolgenerator as symbolgenerator


class MainTest(unittest.TestCase):

    def _call_cabac(self, fun='EGk'):
        num_max_val = 255
        num_bins = 8
        num_values = 1000

        symbols = symbolgenerator.random_uniform(num_values, num_max_val)
        k = 1

        num_ctx = 1
        p1_init = 0.5
        shift_idx = 8

        # Encode
        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctx, p1_init, shift_idx)
        enc.start()

        ctx_ids = [0] * (num_max_val+2)
        for symbol in symbols:
            if fun == 'BIbypass':
                enc.encodeBinsBIbypass(symbol, num_bins)
            elif fun == 'BI':
                enc.encodeBinsBI(symbol, ctx_ids, num_bins)
            elif fun == 'TUbypass':
                enc.encodeBinsTUbypass(symbol, num_max_val)
            elif fun == 'TU':
                enc.encodeBinsTU(symbol, ctx_ids, num_max_val)
            elif fun == 'EGkbypass':
                enc.encodeBinsEGkbypass(symbol, k)
            elif fun == 'EGk':
                enc.encodeBinsEGk(symbol, k, ctx_ids)

        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        # Decode
        dec = cabac.cabacSimpleSequenceDecoder(bs)
        dec.initCtx(num_ctx, p1_init, shift_idx)
        dec.start()

        decodedSymbols = []
        ctx_ids = [0] * (num_max_val+2)
        for _ in range(0, num_values):
            if fun == 'BIbypass':
                decodedSymbol = dec.decodeBinsBIbypass(num_bins)
            elif fun == 'BI':
                decodedSymbol = dec.decodeBinsBI(ctx_ids, num_bins)
            elif fun == 'TUbypass':
                decodedSymbol = dec.decodeBinsTUbypass(num_max_val)
            elif fun == 'TU':
                decodedSymbol = dec.decodeBinsTU(ctx_ids, num_max_val)
            elif fun == 'EGkbypass':
                decodedSymbol = dec.decodeBinsEGkbypass(k)
            elif fun == 'EGk':
                decodedSymbol = dec.decodeBinsEGk(k, ctx_ids)
            else:
                raise Exception('Unknown function: ' + fun)

            decodedSymbols.append(decodedSymbol)

        dec.decodeBinTrm()
        dec.finish()
        print('bitstream length: ' + str(len(bs)))

        self.assertTrue((decodedSymbols == symbols))

    def test_encode_symbols(self):
        random.seed(0)
        print('test_encode_symbols')
        funs = ['BIbypass', 'BI', 'TUbypass', 'TU', 'EGkbypass', 'EGk']

        for fun in funs:
            print('Testing function: ' + fun)
            self._call_cabac(fun)

    def test_ep(self):
        symbol = random.randint(0, 511)
        Nbits = math.ceil(math.log2(symbol))

        # Encoder
        enc = cabac.cabacEncoder()
        enc.start()
        enc.encodeBinsEP(symbol, Nbits)
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        # Decoder
        dec = cabac.cabacDecoder(bs)
        dec.start()
        decodedSymbol = dec.decodeBinsEP(Nbits)
        dec.decodeBinTrm()
        dec.finish()

        self.assertTrue(decodedSymbol == symbol)

    def test_enc_dec(self):

        p1_init = 0.6
        shift_idx = 8
        bitsToEncode = symbolgenerator.random_uniform(1000, 2)

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
