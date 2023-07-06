import unittest
import random
import cabac
import math
import time
import numpy as np

import tests.utils.symbolgenerator as symbolgenerator
import tests.utils.binarization as binarization
import tests.utils.contextselector as contextselector

class MainTest(unittest.TestCase):
    def _tic(self):
        return time.time()

    def _toc(self,t):
        return time.time() - t

    def _call_cabac_order1(self, fun='EGk'):
        
        # Parameters
        ctx_rest_pos = 8
        ctx_symbol_max = 16
        ctx_order = 1
        ctx_offset = 0
        num_max_val = 255
        num_max_prefix_val = int(math.floor(math.log2(num_max_val/(2**0) + 1)) + 1)
        num_bi_bins = 8
        num_symbols = 1000
        #num_ctxs = 3*rest_pos + 1 # binsOrder1
        num_ctxs = (ctx_symbol_max+1)*ctx_rest_pos + 1  # symbolOrder1
        k = 1

        # Symbols
        symbols = symbolgenerator.create_random_symbols_uniform_distribution(num_symbols, num_max_val)
        #symbols = symbolgenerator.create_random_symbols_geometric_distribution(num_values, 0.05)
        #symbols = list(range(0,8))

        num_symbols = len(symbols)
        
        # 1. Binarize each symbol and encode each bin in Python
        enc = cabac.cabacEncoder()
        enc.initCtx(num_ctxs, 0.5, 8) # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()

        t = self._tic()
        symbol_prev = 0
        for i, symbol in enumerate(symbols):
            if i != 0:
                symbol_prev = symbols[i-1]
            
            # Binarization
            if fun == 'TUBAC' or 'TUBinPos' or 'TUbinsOrder1' or fun == 'TUsymbolOrder1':
                bins = binarization.encode_tu(symbol, num_max_val)

            # Encode each bin
            for n, bin in enumerate(bins):
                # Context selection
                if fun == 'TUBAC':
                    ctx_id = 0
                elif fun == 'TUBinPos':
                    ctx_id = contextselector.ctx_id_bin_pos(n=n, n_rst=ctx_rest_pos)
                elif fun == 'TUbinsOrder1':
                    ctx_id = contextselector.ctx_id_bins_order_1_tu(n=n, prev_symbol=symbol_prev, n_rst=ctx_rest_pos)
                elif fun == 'TUsymbolOrder1':
                    ctx_id = contextselector.ctx_id_symbol_order_1_tu(n=n, prev_symbol=symbol_prev, rest_pos=ctx_rest_pos, symbol_max=ctx_symbol_max)
                # Encode
                enc.encodeBin(bin, ctx_id)

        t_enc1 = self._toc(t)
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs1 = enc.getBitstream()

        # Decode
        dec = cabac.cabacDecoder(bs1)
        dec.initCtx(num_ctxs, 0.5, 8)
        dec.start()

        t = self._tic()
        symbols_dec = np.zeros(num_symbols, dtype=np.uint64)
        symbol_prev_dec = 0
        for i in range(0, num_symbols):
            symbol = 0
            n = 0
            if fun == 'TUBAC':
                ctx_id = 0
            elif fun == 'TUBinPos':
                ctx_id = contextselector.ctx_id_bin_pos(n=n, n_rst=ctx_rest_pos)
            elif fun == 'TUbinsOrder1':
                ctx_id = contextselector.ctx_id_bins_order_1_tu(n=n, prev_symbol=symbol_prev_dec, n_rst=ctx_rest_pos)
            elif fun == 'TUsymbolOrder1':
                ctx_id = contextselector.ctx_id_symbol_order_1_tu(n=n, prev_symbol=symbol_prev_dec, rest_pos=ctx_rest_pos, symbol_max=ctx_symbol_max)
            while dec.decodeBin(ctx_id) == 1:
                symbol += 1
                n += 1
                if fun == 'TUBAC':
                    ctx_id = 0
                elif fun == 'TUBinPos':
                    ctx_id = contextselector.ctx_id_bin_pos(n=n, n_rst=ctx_rest_pos)
                elif fun == 'TUbinsOrder1':
                    ctx_id = contextselector.ctx_id_bins_order_1_tu(n=n, prev_symbol=symbol_prev_dec, n_rst=ctx_rest_pos)
                elif fun == 'TUsymbolOrder1':
                    ctx_id = contextselector.ctx_id_symbol_order_1_tu(n=n, prev_symbol=symbol_prev_dec, rest_pos=ctx_rest_pos, symbol_max=ctx_symbol_max)
            symbols_dec[i] = symbol
            symbol_prev_dec = symbol
        
        t_dec1 = self._toc(t)

        dec.decodeBinTrm()
        dec.finish()

        print('Bitstream length: ' + str(len(bs1)))

        self.assertTrue((symbols_dec == symbols).all())

        ############################################################
        # 2. Encode each symbol in Python
        ############################################################
        # 2.a) with context IDs passed by array
        
        enc = cabac.cabacSymbolEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()
        t = self._tic()
        n_array = np.arange(0, num_max_val+1)
        symbol_prev = 0
        for i, symbol in enumerate(symbols):
            if fun == 'TUBAC':
                ctx_ids = np.zeros(n_array.shape, dtype=np.int32)
            elif fun == 'TUBinPos':
                ctx_ids = contextselector.ctx_ids_bin_pos(n=n_array, n_rst=ctx_rest_pos)
            elif fun == 'TUbinsOrder1':
                ctx_ids = contextselector.ctx_ids_bins_order_1_tu(n=n_array, prev_symbol=symbol_prev, n_rst=ctx_rest_pos)
            elif fun == 'TUsymbolOrder1':
                ctx_ids = contextselector.ctx_ids_symbol_order_1_tu(n=n_array, prev_symbol=symbol_prev, rest_pos=ctx_rest_pos, symbol_max=ctx_symbol_max)
            enc.encodeBinsTU(symbol, ctx_ids, num_max_val)
            symbol_prev = symbol

        
        t_enc2a = self._toc(t)
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs2a = enc.getBitstream()

        # Decode
        dec = cabac.cabacSymbolDecoder(bs2a)
        dec.initCtx(num_ctxs, 0.5, 8) 
        dec.start()

        t = self._tic()
        symbols_dec = np.zeros(num_symbols, dtype=np.uint64)
        symbol_prev_dec = 0
        for i in range(0, num_symbols):
            if fun == 'TUBAC':
                ctx_ids = np.zeros(n_array.shape, dtype=np.int32)
            elif fun == 'TUBinPos':
                ctx_ids = contextselector.ctx_ids_bin_pos(n=n_array, n_rst=ctx_rest_pos)
            elif fun == 'TUbinsOrder1':
                ctx_ids = contextselector.ctx_ids_bins_order_1_tu(n=n_array, prev_symbol=symbol_prev_dec, n_rst=ctx_rest_pos)
            elif fun == 'TUsymbolOrder1':
                ctx_ids = contextselector.ctx_ids_symbol_order_1_tu(n=n_array, prev_symbol=symbol_prev_dec, rest_pos=ctx_rest_pos, symbol_max=ctx_symbol_max)
           
            symbol_dec = dec.decodeBinsTU(ctx_ids, num_max_val)
            symbol_prev_dec = symbol_dec

            symbols_dec[i] = symbol_dec

        t_dec2a = self._toc(t)

        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs2a)))

        self.assertTrue((symbols_dec == symbols).all())
        self.assertTrue(t_enc1 > t_enc2a)
        self.assertTrue(t_dec1 > t_dec2a)
        self.assertTrue(bs1 == bs2a)


        ############################################################
        # 2.b) with context IDs passed by Python function
        
        enc = cabac.cabacSymbolEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()
        t = self._tic()
        n_array = np.arange(0, num_max_val+1)
        symbol_prev = 0
        for i, symbol in enumerate(symbols):
            if fun == 'TUBAC':
                ctx_fun = lambda n: 0
            elif fun == 'TUBinPos':
                ctx_fun = lambda n: contextselector.ctx_id_bin_pos(n=n, n_rst=ctx_rest_pos)
            elif fun == 'TUbinsOrder1':
                ctx_fun = lambda n: contextselector.ctx_id_bins_order_1_tu(n=n, prev_symbol=symbol_prev, n_rst=ctx_rest_pos)
            elif fun == 'TUsymbolOrder1':
                ctx_fun = lambda n: contextselector.ctx_id_symbol_order_1_tu(n=n, prev_symbol=symbol_prev, rest_pos=ctx_rest_pos, symbol_max=ctx_symbol_max)
            enc.encodeBinsTU(symbol, ctx_fun, num_max_val)
            symbol_prev = symbol

        
        t_enc2b = self._toc(t)
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs2b = enc.getBitstream()

        # Decode
        dec = cabac.cabacSymbolDecoder(bs2b)
        dec.initCtx(num_ctxs, 0.5, 8) 
        dec.start()

        t = self._tic()
        symbols_dec = np.zeros(num_symbols, dtype=np.uint64)
        symbol_prev_dec = 0
        for i in range(0, num_symbols):            
            if fun == 'TUBAC':
                ctx_fun = lambda n: 0
            elif fun == 'TUBinPos':
                ctx_fun = lambda n: contextselector.ctx_id_bin_pos(n=n, n_rst=ctx_rest_pos)
            elif fun == 'TUbinsOrder1':
                ctx_fun = lambda n: contextselector.ctx_id_bins_order_1_tu(n=n, prev_symbol=symbol_prev_dec, n_rst=ctx_rest_pos)
            elif fun == 'TUsymbolOrder1':
                ctx_fun = lambda n: contextselector.ctx_id_symbol_order_1_tu(n=n, prev_symbol=symbol_prev_dec, rest_pos=ctx_rest_pos, symbol_max=ctx_symbol_max)

            symbol_dec = dec.decodeBinsTU(ctx_fun, num_max_val)
            symbol_prev_dec = symbol_dec

            symbols_dec[i] = symbol_dec

        t_dec2b = self._toc(t)

        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs2b)))

        self.assertTrue((symbols_dec == symbols).all())
        self.assertTrue(t_enc2a < t_enc2b)
        self.assertTrue(t_dec2a < t_dec2b)
        self.assertTrue(bs1 == bs2b)


        ############################################################
        # 2.c) with context IDs passed by C++ function
        
        enc = cabac.cabacSymbolEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()
        t = self._tic()
        n_array = np.arange(0, num_max_val+1)
        symbol_prev = 0
        for i, symbol in enumerate(symbols):
            if fun == 'TUBAC':
                ctx_fun = lambda n: 0
            elif fun == 'TUBinPos':
                ctx_fun = lambda n: cabac.getContextIdBinPosition(n, ctx_rest_pos)
            elif fun == 'TUbinsOrder1':
                ctx_fun = lambda n: cabac.getContextIdBinsOrder1TU(n, symbol_prev, ctx_rest_pos)
            elif fun == 'TUsymbolOrder1':
                ctx_fun = lambda n: cabac.getContextIdSymbolOrder1TU(n, symbol_prev, ctx_rest_pos, ctx_symbol_max)
            enc.encodeBinsTU(symbol, ctx_fun, num_max_val)
            symbol_prev = symbol

        
        t_enc2c = self._toc(t)
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs2c = enc.getBitstream()

        # Decode
        dec = cabac.cabacSymbolDecoder(bs2c)
        dec.initCtx(num_ctxs, 0.5, 8) 
        dec.start()

        t = self._tic()
        symbols_dec = np.zeros(num_symbols, dtype=np.uint64)
        symbol_prev_dec = 0
        for i in range(0, num_symbols):            
            if fun == 'TUBAC':
                ctx_fun = lambda n: 0
            elif fun == 'TUBinPos':
                ctx_fun = lambda n: cabac.getContextIdBinPosition(n, ctx_rest_pos)
            elif fun == 'TUbinsOrder1':
                ctx_fun = lambda n: cabac.getContextIdBinsOrder1TU(n, symbol_prev_dec, ctx_rest_pos)
            elif fun == 'TUsymbolOrder1':
                ctx_fun = lambda n: cabac.getContextIdSymbolOrder1TU(n, symbol_prev_dec, ctx_rest_pos, ctx_symbol_max)

            symbol_dec = dec.decodeBinsTU(ctx_fun, num_max_val)
            symbol_prev_dec = symbol_dec

            symbols_dec[i] = symbol_dec

        t_dec2c = self._toc(t)

        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs2c)))

        self.assertTrue((symbols_dec == symbols).all())
        self.assertTrue(t_enc2a < t_enc2c)
        self.assertTrue(t_dec2a < t_dec2c)
        self.assertTrue(bs1 == bs2c)
        


        ############################################################
        # 3. Encode sequence
        ############################################################

        bin_params = [num_max_val]
        ctx_params = [ctx_order, ctx_rest_pos, ctx_offset, ctx_symbol_max]
        bin_id = cabac.BinarizationId.TU
        if fun == 'TUBAC':
            ctx_model_id = cabac.ContextModelId.BAC
        elif fun == 'TUBinPos':
            ctx_model_id = cabac.ContextModelId.BINPOSITION
        elif fun == 'TUbinsOrder1':
            ctx_model_id = cabac.ContextModelId.BINSORDERN
        elif fun == 'TUsymbolOrder1':
            ctx_model_id = cabac.ContextModelId.SYMBOLORDERN

        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()
        t = self._tic()
        enc.encodeSymbols(symbols, bin_id, ctx_model_id, bin_params, ctx_params)
        t_enc3 = self._toc(t)

        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs3 = enc.getBitstream()

        # Decode
        dec = cabac.cabacSimpleSequenceDecoder(bs3)
        dec.initCtx(num_ctxs, 0.5, 8)
        dec.start()

        t = self._tic()
        symbols_dec = dec.decodeSymbols(len(symbols), bin_id, ctx_model_id, bin_params, ctx_params)
        t_dec3 = self._toc(t)

        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs3)))

        self.assertTrue((symbols_dec == symbols).all())
        self.assertTrue(t_enc2a > t_enc3)
        self.assertTrue(t_dec2a > t_dec3)
        self.assertTrue(bs1 == bs3)

    def test_encode_symbols_timing(self):
        random.seed(0)
        print('test_encode_symbols_symbol_order1')
        funs = ['TUBAC', 'TUBinPos', 'TUbinsOrder1', 'TUsymbolOrder1']

        for fun in funs:
            print('Testing function: ' + fun)
            self._call_cabac_order1(fun)


if __name__ == "__main__":
    unittest.main()
