import unittest
import random
import cabac
import math


def create_random_symbols_geometric_distribution(num_values, p):
    import numpy as np

    return (np.random.default_rng(seed=0).geometric(p, num_values) - 1).tolist()


class MainTest(unittest.TestCase):
    

    def _call_cabac_order1(self, fun='EG0'):
        
        rest_pos = 8
        symbol_max = 16
        num_max_val = 255
        num_max_prefix_val = int(math.floor(math.log2(num_max_val/(2**0) + 1)) + 1)
        num_bins = 8
        num_values = 1000
        #num_ctxs = 3*rest_pos + 1 # binsOrder1
        num_ctxs = (symbol_max+1)*rest_pos + 1  # symbolOrder1
        k = 1
        symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        symbols = create_random_symbols_geometric_distribution(num_values, 0.05)
        #symbols = [0,1,2,3,4,5,6,7]
        #num_values = len(symbols)
        

        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()

        symbolPrev = 0
        for i, symbol in enumerate(symbols):
            if i != 0:
                symbolPrev = symbols[i-1]
        
            if fun == 'BIbypass':
                enc.encodeBinsBIbypass(symbol, num_bins)
            elif fun == 'TUbypass':
                enc.encodeBinsTUbypass(symbol, num_max_val)
            elif fun == 'TUbinsOrder1':
                enc.encodeBinsTUbinsOrder1(symbol, symbolPrev, rest_pos, num_max_val)
            elif fun == 'TUsymbolOrder1':
                enc.encodeBinsTUsymbolOrder1(symbol, symbolPrev, rest_pos, symbol_max, num_max_val)
            elif fun == 'EG0bypass':
                enc.encodeBinsEG0bypass(symbol)
            elif fun == 'EG0binsOrder1':
                enc.encodeBinsEG0binsOrder1(symbol, symbolPrev, rest_pos, num_max_prefix_val)
            elif fun == 'EG0symbolOrder1':
                enc.encodeBinsEG0symbolOrder1(symbol, symbolPrev, rest_pos, symbol_max, num_max_prefix_val)
            elif fun == 'EGkbypass':
                enc.encodeBinsEGkbypass(symbol, k)
            elif fun == 'EGkbinsOrder1':
                enc.encodeBinsEGkbinsOrder1(symbol, symbolPrev, k, rest_pos, num_max_prefix_val)
            elif fun == 'EGksymbolOrder1':
                enc.encodeBinsEGksymbolOrder1(symbol, symbolPrev, k, rest_pos, symbol_max, num_max_prefix_val)
                    
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        # Decode
        dec = cabac.cabacSimpleSequenceDecoder(bs)
        dec.initCtx(num_ctxs, 0.5, 8) 
        dec.start()

        decodedSymbols = []
        decodedSymbolPrev = 0
        for i in range(0, num_values):
            if i != 0:
                decodedSymbolPrev = decodedSymbols[i-1]
            
            if fun == 'BIbypass':
                decodedSymbol = dec.decodeBinsBIbypass(num_bins)
            elif fun == 'TUbypass':
                decodedSymbol = dec.decodeBinsTUbypass(num_max_val)
            elif fun == 'TUbinsOrder1':
                decodedSymbol = dec.decodeBinsTUbinsOrder1(decodedSymbolPrev, rest_pos, num_max_val)
            elif fun == 'TUsymbolOrder1':
                decodedSymbol = dec.decodeBinsTUsymbolOrder1(decodedSymbolPrev, rest_pos, symbol_max, num_max_val)
            elif fun == 'EG0bypass':
                decodedSymbol = dec.decodeBinsEG0bypass()
            elif fun == 'EG0binsOrder1':
                decodedSymbol = dec.decodeBinsEG0binsOrder1(decodedSymbolPrev, rest_pos, num_max_prefix_val)
            elif fun == 'EG0symbolOrder1':
                decodedSymbol = dec.decodeBinsEG0symbolOrder1(decodedSymbolPrev, rest_pos, symbol_max, num_max_prefix_val)
            elif fun == 'EGkbypass':
                decodedSymbol = dec.decodeBinsEGkbypass(k)
            elif fun == 'EGkbinsOrder1':
                decodedSymbol = dec.decodeBinsEGkbinsOrder1(decodedSymbolPrev, k, rest_pos, num_max_prefix_val)
            elif fun == 'EGksymbolOrder1':
                decodedSymbol = dec.decodeBinsEGksymbolOrder1(decodedSymbolPrev, k, rest_pos, symbol_max, num_max_prefix_val)

            decodedSymbols.append(decodedSymbol)
        
        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs)))

        self.assertTrue((decodedSymbols == symbols))

    def test_encode_symbols_order1(self):
        random.seed(0)
        print('test_encode_symbols_symbol_order1')
        funs = ['BIbypass', 'TUbypass', 'TUbinsOrder1', 'TUsymbolOrder1', 'EG0bypass', 'EG0binsOrder1', 'EG0symbolOrder1', 'EGkbypass', 'EGkbinsOrder1', 'EGksymbolOrder1']

        for fun in funs:
            print('Testing function: ' + fun)
            self._call_cabac_order1(fun)

    def _call_cabac(self, fun='EG0'):
        num_max_val = 255
        num_bins = 8
        num_values = 1000
        num_ctx = 1
        symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        k = 1
        

        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctx, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
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
            elif fun == 'EG0bypass':
                enc.encodeBinsEG0bypass(symbol)
            elif fun == 'EG0':
                enc.encodeBinsEG0(symbol, ctx_ids)
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
        dec.initCtx(num_ctx, 0.5, 8) 
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
            elif fun == 'EG0bypass':
                decodedSymbol = dec.decodeBinsEG0bypass()
            elif fun == 'EG0':
                decodedSymbol = dec.decodeBinsEG0(ctx_ids)
            elif fun == 'EGkbypass':
                decodedSymbol = dec.decodeBinsEGkbypass(k)
            elif fun == 'EGk':
                decodedSymbol = dec.decodeBinsEGk(k, ctx_ids)

            decodedSymbols.append(decodedSymbol)
        
        dec.decodeBinTrm()
        dec.finish()
        print('bitstream length: ' + str(len(bs)))

        self.assertTrue((decodedSymbols == symbols))

    def test_encode_symbols(self):
        random.seed(0)
        print('test_encode_symbols')
        funs = ['BIbypass', 'BI', 'TUbypass', 'TU', 'EG0bypass', 'EG0', 'EGkbypass', 'EGk']

        for fun in funs:
            print('Testing function: ' + fun)
            self._call_cabac(fun)

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
