import unittest
import random
import cabac
import math


class MainTest(unittest.TestCase):

    def _call_cabac_order1(self, fun='EG0'):
        
        rest_pos = 10
        num_max_val = 255
        num_max_prefix_val = int(math.floor(math.log2(num_max_val/(2**0) + 1)) + 1)
        num_bins = 8
        num_values = 1000
        num_ctxs = 3*rest_pos + 1
        k = 1
        symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
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
            #elif fun == 'BI':
            #    enc.encodeBinsBIorder1(symbol, symbolPrev, num_bins, rest_pos)
            elif fun == 'TUbypass':
                enc.encodeBinsTUbypass(symbol, num_max_val)
            elif fun == 'TU':
                enc.encodeBinsTUorder1(symbol, symbolPrev, rest_pos, num_max_val)
            elif fun == 'EG0bypass':
                enc.encodeBinsEG0bypass(symbol)
            elif fun == 'EG0':
                enc.encodeBinsEG0order1(symbol, symbolPrev, rest_pos, num_max_prefix_val)
            elif fun == 'EGkbypass':
                enc.encodeBinsEGkbypass(symbol, k)
            elif fun == 'EGk':
                enc.encodeBinsEGkorder1(symbol, symbolPrev, k, rest_pos, num_max_prefix_val)

        
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
            #elif fun == 'BI':
                #decodedSymbol = dec.decodeBinsBIorder1(decodedSymbolPrev, num_bins, rest_pos)
            elif fun == 'TUbypass':
                decodedSymbol = dec.decodeBinsTUbypass(num_max_val)
            elif fun == 'TU':
                decodedSymbol = dec.decodeBinsTUorder1(decodedSymbolPrev, rest_pos, num_max_val)
            elif fun == 'EG0bypass':
                decodedSymbol = dec.decodeBinsEG0bypass()
            elif fun == 'EG0':
                decodedSymbol = dec.decodeBinsEG0order1(decodedSymbolPrev, rest_pos, num_max_prefix_val)
            elif fun == 'EGkbypass':
                decodedSymbol = dec.decodeBinsEGkbypass(k)
            elif fun == 'EGk':
                decodedSymbol = dec.decodeBinsEGkorder1(decodedSymbolPrev, k, rest_pos, num_max_prefix_val)

            decodedSymbols.append(decodedSymbol)
        
        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs)))

        self.assertTrue((decodedSymbols == symbols))

    def test_encode_symbols_order1(self):
        random.seed(0)
        print('test_encode_symbols_order1')
        funs = ['BIbypass', 'TUbypass', 'TU', 'EG0bypass', 'EG0', 'EGkbypass', 'EGk']

        for fun in funs:
            print('Testing function: ' + fun)
            self._call_cabac_order1(fun)

    def test_ctx_selection_bins_order1(self):
        import numpy as np  # TODO: move this import somewhere else
        def ctx_ids_bins_order1_tu(n, prev_symbol=0, n_rst=10):
            # n is the position of the to-be coded bin in the bin string (of the to be coded symbol)
            # prev_symbol is the previously coded symbol
            # bins with n < n_rst are coded with ctx model, all other bins are coded with the same context.
            # returns the context id
            #
            # First ctx_ids correspond to previous bin at position n not available case.
            # Next ctx_ids correspond to previous bin at position n = 0 case.
            # Next ctx_ids correspond to previous bin at position n = 1 case.
            # Last ctx_id is used for all bins n >= n_rst

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
        
        num_max_val = 255
        num_values = 1000
        symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        
        symbolPrev = 0
        num_rest = 10
        for i, symbol in enumerate(symbols):
            if i != 0:
                symbolPrev = symbols[i-1]
            
            n = list(range(0, symbol+1))
            ctx_ids = ctx_ids_bins_order1_tu(n, symbolPrev, num_rest)
            ctx_ids2 = cabac.getContextIdsBinsOrder1TU(symbolPrev, num_rest, symbol+1)
            self.assertTrue((ctx_ids == np.array(ctx_ids2)).all())

    def test_ctx_selection_symbols_order1(self):
        import numpy as np  # TODO: move this import somewhere else
        def ctx_ids_symbols_order1_tu(n, prev_symbol=0, rest_pos=10, symbol_max=32):
            # ContextID dependent on bin position n of to-be-decoded symbol as well as previous integer symbol value, in fact
            # modeling the probability p(b_n | symbolPrev) with bin b_n at position n in TU-binarized bin-string.

            # In total we have num_ctx_total = (symbolMax+2)*restPos + 1 contexts
    # 
            # The ctx_id is computed as follows:
            # ctx_id:                       meaning:
            # 0*restPos ... 1*restPos-1:    previously coded symbol = 0 (and n<restPos)
            # 1*restPos ... 2*restPos-1:    previously coded symbol = 1 (and n<restPos)
            # ...
            # (symbolMax+0)*restPos ... (symbolMax+1)*restPos-1:  previously coded symbol = symbolMax (and n<restPos)
            # (symbolMax+1)*restPos ... (symbolMax+2)*restPos-1:  previously coded symbol > symbolMax (and n<restPos)
            # (symbolMax+2)*restPos:                              position n>=restPos: rest case with single context

            n = np.asarray(n)
            ctx_id = np.zeros(n.shape, dtype=np.int32)

            # binary mask
            mask_not_rest = n < rest_pos  # mask for not-rest case

            # clip previous symbol to symbol_max + 1
            prev_symbol0 = prev_symbol if prev_symbol <= symbol_max else symbol_max+1
            
            # bin position n < rest_pos
            ctx_id[mask_not_rest] = (prev_symbol0)*rest_pos + n[mask_not_rest]

            # bin position n >= rest_pos
            # rst case, independent of position n
            ctx_id[~mask_not_rest] = (symbol_max+2)*rest_pos

            return ctx_id
        
        num_max_val = 255
        num_values = 1000
        symbol_max = 15
        symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        
        symbolPrev = 0
        num_rest = 10
        for i, symbol in enumerate(symbols):
            if i != 0:
                symbolPrev = symbols[i-1]
            
            n = list(range(0, symbol+1))
            ctx_ids = ctx_ids_symbols_order1_tu(n, symbolPrev, num_rest, symbol_max)
            ctx_ids2 = cabac.getContextIdsSymbolOrder1TU(symbolPrev, num_rest, symbol_max, symbol+1)
            self.assertTrue((ctx_ids == np.array(ctx_ids2)).all())

    def _call_cabac(self, fun='EG0'):
        num_max_val = 255
        num_bins = 8
        num_values = 1000
        num_ctx = 1
        symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        

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
            decodedSymbols.append(decodedSymbol)
        
        dec.decodeBinTrm()
        dec.finish()
        print('bitstream length: ' + str(len(bs)))

        self.assertTrue((decodedSymbols == symbols))

    def test_encode_symbols(self):
        random.seed(0)
        print('test_encode_symbols')
        funs = ['BIbypass', 'BI', 'TUbypass', 'TU', 'EG0bypass', 'EG0']

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
