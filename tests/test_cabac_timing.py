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
        
        # Binarization ID
        if fun == 'TUBAC' or fun == 'TUBinPos' or fun == 'TUbinsOrder1' or fun == 'TUsymbolOrder1':
            bin_id = cabac.BinarizationId.TU
        elif fun == 'EGkBAC' or fun == 'EGkBinPos' or fun == 'EGkbinsOrder1' or fun == 'EGksymbolOrder1':
            bin_id = cabac.BinarizationId.EGk

        # Parameters
        ctx_rest_pos = 8
        ctx_symbol_max = 16
        ctx_order = 1
        ctx_offset = 0
        num_max_val = 255
        num_max_prefix_val = int(math.floor(math.log2(num_max_val/(2**0) + 1)) + 1)
        num_bi_bins = 8
        num_symbols = 5000
        #num_ctxs = 3*rest_pos + 1 # binsOrder1
        num_ctxs = (ctx_symbol_max+1)*ctx_rest_pos + 1  # symbolOrder1
        k = 1

        # 0. Create symbol sequence
        symbols = symbolgenerator.create_random_symbols_uniform_distribution(num_symbols, num_max_val)
        #symbols = symbolgenerator.create_random_symbols_geometric_distribution(num_values, 0.05)
        #symbols = list(range(0,8))

        num_symbols = len(symbols)
        
        # 1. Binarize each symbol and encode each bin in Python

        def ctx_id_wrapper(fun, n, symbol_prev, ctx_rest_pos, ctx_symbol_max, k=0):
            if fun == 'TUBAC' or fun == 'EGkBAC':
                ctx_id = 0
            elif fun == 'TUBinPos' or fun == 'EGkBinPos': 
                ctx_id = contextselector.ctx_id_bin_pos(n=n, n_rst=ctx_rest_pos)
            elif fun == 'TUbinsOrder1':
                ctx_id = contextselector.ctx_id_bins_order_1_tu(n=n, prev_symbol=symbol_prev, n_rst=ctx_rest_pos)
            elif fun == 'TUsymbolOrder1':
                ctx_id = contextselector.ctx_id_symbol_order_1_tu(n=n, prev_symbol=symbol_prev, rest_pos=ctx_rest_pos, symbol_max=ctx_symbol_max)
            elif fun == 'EGkbinsOrder1':
                ctx_id = contextselector.ctx_id_bins_order_1_egk(n=n, prev_symbol=symbol_prev, n_rst=ctx_rest_pos, k=k)
            elif fun == 'EGksymbolOrder1':
                ctx_id = contextselector.ctx_id_symbol_order_1_egk(n=n, prev_symbol=symbol_prev, rest_pos=ctx_rest_pos, symbol_max=ctx_symbol_max, k=k)

            return ctx_id


        # Encode
        enc = cabac.cabacEncoder()
        enc.initCtx(num_ctxs, 0.5, 8) # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()

        t = self._tic()
        symbol_prev = 0
        for i, symbol in enumerate(symbols):
            if i != 0:
                symbol_prev = symbols[i-1]
            
            # Binarization
            if bin_id == cabac.BinarizationId.TU:
                bins = binarization.encode_tu(symbol, num_max_val)
            elif bin_id == cabac.BinarizationId.EGk:
                _, bins, bins_suffix = binarization.encode_eg(symbol, k, return_prefix_suffix=True)

            # Encode each bin
            for n, bin in enumerate(bins):
                # Context selection
                ctx_id = ctx_id_wrapper(fun, n, symbol_prev, ctx_rest_pos, ctx_symbol_max, k)

                # Encode
                enc.encodeBin(bin, ctx_id)
            
            if bin_id == cabac.BinarizationId.EGk:
                # Encode suffix
                for n, bin in enumerate(bins_suffix):
                    enc.encodeBinEP(bin)

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

        # TU encodes integer value v as a sequence of '1' of length v, terminated by a '0'
        # EGk encodes its prefix p as a sequence of '0', terminated by a '1'
        if bin_id == cabac.BinarizationId.TU:
            bin_value_to_check = 1
        elif bin_id == cabac.BinarizationId.EGk:
            bin_value_to_check = 0

        for i in range(0, num_symbols):
            symbol = 0
            n = 0
            ctx_id = ctx_id_wrapper(fun, n, symbol_prev_dec, ctx_rest_pos, ctx_symbol_max, k)

            # TU-encoded value or EGk-encoded prefix
            while dec.decodeBin(ctx_id) == bin_value_to_check:
                symbol += 1
                n += 1

                ctx_id = ctx_id_wrapper(fun, n, symbol_prev_dec, ctx_rest_pos, ctx_symbol_max, k)
            
            # For EGk, decode suffix and then symbol
            if bin_id == cabac.BinarizationId.EGk:
                numLeadZeros = symbol
                m = (1 << (numLeadZeros + k)) - (1 << k)
                symbol = dec.decodeBinsEP(numLeadZeros + k)
                symbol = symbol + m

            symbols_dec[i] = symbol
            symbol_prev_dec = symbol
        
        t_dec1 = self._toc(t)

        dec.decodeBinTrm()
        dec.finish()

        self.assertTrue((symbols_dec == symbols).all())


        ############################################################
        # 2. Encode each symbol in Python
        ############################################################
        # 2.a) with context IDs passed by array
        
        def ctx_ids_wrapper(fun, n_array, symbol_prev, ctx_rest_pos, ctx_symbol_max, k=0):
            if fun == 'TUBAC' or fun == 'EGkBAC':
                ctx_ids = np.zeros(n_array.shape, dtype=np.int32)
            elif fun == 'TUBinPos' or fun == 'EGkBinPos':
                ctx_ids = contextselector.ctx_ids_bin_pos(n=n_array, n_rst=ctx_rest_pos)
            elif fun == 'TUbinsOrder1':
                ctx_ids = contextselector.ctx_ids_bins_order_1_tu(n=n_array, prev_symbol=symbol_prev, n_rst=ctx_rest_pos)
            elif fun == 'TUsymbolOrder1':
                ctx_ids = contextselector.ctx_ids_symbol_order_1_tu(n=n_array, prev_symbol=symbol_prev, rest_pos=ctx_rest_pos, symbol_max=ctx_symbol_max)
            elif fun == 'EGkbinsOrder1':
                ctx_ids = contextselector.ctx_ids_bins_order_1_egk(n=n_array, prev_symbol=symbol_prev, n_rst=ctx_rest_pos, k=k)
            elif fun == 'EGksymbolOrder1':
                ctx_ids = contextselector.ctx_ids_symbol_order_1_egk(n=n_array, prev_symbol=symbol_prev, rest_pos=ctx_rest_pos, symbol_max=ctx_symbol_max, k=k)

            return ctx_ids

        # Encode
        enc = cabac.cabacSymbolEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()
        t = self._tic()
        n_array = np.arange(0, num_max_val+1)
        symbol_prev = 0
        for i, symbol in enumerate(symbols):
            ctx_ids = ctx_ids_wrapper(fun, n_array, symbol_prev, ctx_rest_pos, ctx_symbol_max, k)
            if bin_id == cabac.BinarizationId.TU:
                enc.encodeBinsTU(symbol, ctx_ids, num_max_val)
            elif bin_id == cabac.BinarizationId.EGk:
                enc.encodeBinsEGk(symbol, k, ctx_ids)
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
            ctx_ids = ctx_ids_wrapper(fun, n_array, symbol_prev_dec, ctx_rest_pos, ctx_symbol_max, k)
           
            if bin_id == cabac.BinarizationId.TU:
                symbol_dec = dec.decodeBinsTU(ctx_ids, num_max_val)
            elif bin_id == cabac.BinarizationId.EGk:
                symbol_dec = dec.decodeBinsEGk(k, ctx_ids)
            
            symbol_prev_dec = symbol_dec

            symbols_dec[i] = symbol_dec

        t_dec2a = self._toc(t)

        dec.decodeBinTrm()
        dec.finish()

        self.assertTrue((symbols_dec == symbols).all())
        self.assertTrue(bs1 == bs2a)


        ############################################################
        # 2.b) with context IDs passed by Python function
        
        def ctx_fun_wrapper(fun, symbol_prev, ctx_rest_pos, ctx_symbol_max, k=0):
            if fun == 'TUBAC' or fun == 'EGkBAC':
                ctx_fun = lambda n: 0
            elif fun == 'TUBinPos' or fun == 'EGkBinPos':
                ctx_fun = lambda n: contextselector.ctx_id_bin_pos(n=n, n_rst=ctx_rest_pos)
            elif fun == 'TUbinsOrder1':
                ctx_fun = lambda n: contextselector.ctx_id_bins_order_1_tu(n=n, prev_symbol=symbol_prev, n_rst=ctx_rest_pos)
            elif fun == 'TUsymbolOrder1':
                ctx_fun = lambda n: contextselector.ctx_id_symbol_order_1_tu(n=n, prev_symbol=symbol_prev, rest_pos=ctx_rest_pos, symbol_max=ctx_symbol_max)
            elif fun == 'EGkbinsOrder1':
                ctx_fun = lambda n: contextselector.ctx_id_bins_order_1_egk(n=n, prev_symbol=symbol_prev, n_rst=ctx_rest_pos, k=k)
            elif fun == 'EGksymbolOrder1':
                ctx_fun = lambda n: contextselector.ctx_id_symbol_order_1_egk(n=n, prev_symbol=symbol_prev, rest_pos=ctx_rest_pos, symbol_max=ctx_symbol_max, k=k)
            
            return ctx_fun

        # Encode
        enc = cabac.cabacSymbolEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()
        t = self._tic()
        n_array = np.arange(0, num_max_val+1)
        symbol_prev = 0
        for i, symbol in enumerate(symbols):
            ctx_fun = ctx_fun_wrapper(fun=fun, symbol_prev=symbol_prev, ctx_rest_pos=ctx_rest_pos, ctx_symbol_max=ctx_symbol_max, k=k)
            if bin_id == cabac.BinarizationId.TU:
                enc.encodeBinsTU(symbol, ctx_fun, num_max_val)
            elif bin_id == cabac.BinarizationId.EGk:
                enc.encodeBinsEGk(symbol, k, ctx_fun)
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
            ctx_fun = ctx_fun_wrapper(fun=fun, symbol_prev=symbol_prev_dec, ctx_rest_pos=ctx_rest_pos, ctx_symbol_max=ctx_symbol_max, k=k)

            if bin_id == cabac.BinarizationId.TU:
                symbol_dec = dec.decodeBinsTU(ctx_fun, num_max_val)
            elif bin_id == cabac.BinarizationId.EGk:
                symbol_dec = dec.decodeBinsEGk(k, ctx_fun)
            symbol_prev_dec = symbol_dec

            symbols_dec[i] = symbol_dec

        t_dec2b = self._toc(t)

        dec.decodeBinTrm()
        dec.finish()

        self.assertTrue((symbols_dec == symbols).all())
        self.assertTrue(bs1 == bs2b)


        ############################################################
        # 2.c) with context IDs passed by C++ function
        
        def ctx_fun_wrapper(fun, symbol_prev, ctx_rest_pos, ctx_symbol_max, k=0):
            if fun == 'TUBAC' or fun == 'EGkBAC':
                ctx_fun = lambda n: 0
            elif fun == 'TUBinPos'or fun == 'EGkBinPos':
                ctx_fun = lambda n: cabac.getContextIdBinPosition(n, ctx_rest_pos)
            elif fun == 'TUbinsOrder1':
                ctx_fun = lambda n: cabac.getContextIdBinsOrder1TU(n, symbol_prev, ctx_rest_pos)
            elif fun == 'TUsymbolOrder1':
                ctx_fun = lambda n: cabac.getContextIdSymbolOrder1TU(n, symbol_prev, ctx_rest_pos, ctx_symbol_max)
            elif fun == 'EGkbinsOrder1':
                ctx_fun = lambda n: cabac.getContextIdBinsOrder1EGk(n, symbol_prev, k, ctx_rest_pos)
            elif fun == 'EGksymbolOrder1':
                ctx_fun = lambda n: cabac.getContextIdSymbolOrder1EGk(n, symbol_prev, k, ctx_rest_pos, ctx_symbol_max)

            return ctx_fun


        # Encode
        enc = cabac.cabacSymbolEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()
        t = self._tic()
        n_array = np.arange(0, num_max_val+1)
        symbol_prev = 0
        for i, symbol in enumerate(symbols):
            ctx_fun = ctx_fun_wrapper(fun=fun, symbol_prev=symbol_prev, ctx_rest_pos=ctx_rest_pos, ctx_symbol_max=ctx_symbol_max, k=k)
            if bin_id == cabac.BinarizationId.TU:
                enc.encodeBinsTU(symbol, ctx_fun, num_max_val)
            elif bin_id == cabac.BinarizationId.EGk:
                enc.encodeBinsEGk(symbol, k, ctx_fun)
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
            ctx_fun = ctx_fun_wrapper(fun=fun, symbol_prev=symbol_prev_dec, ctx_rest_pos=ctx_rest_pos, ctx_symbol_max=ctx_symbol_max, k=k)
            if bin_id == cabac.BinarizationId.TU:
                symbol_dec = dec.decodeBinsTU(ctx_fun, num_max_val)
            elif bin_id == cabac.BinarizationId.EGk:
                symbol_dec = dec.decodeBinsEGk(k, ctx_fun)
            symbol_prev_dec = symbol_dec

            symbols_dec[i] = symbol_dec

        t_dec2c = self._toc(t)

        dec.decodeBinTrm()
        dec.finish()

        self.assertTrue((symbols_dec == symbols).all())
        self.assertTrue(bs1 == bs2c)


        ############################################################
        # 3. Encode sequence
        ############################################################
        # 3.a) Encode whole sequence in one pass

        bin_params = [num_max_val, k]
        ctx_params = [ctx_order, ctx_rest_pos, ctx_offset, ctx_symbol_max]

        if fun == 'TUBAC' or fun == 'EGkBAC':
            ctx_model_id = cabac.ContextModelId.BAC
        elif fun == 'TUBinPos' or fun == 'EGkBinPos':
            ctx_model_id = cabac.ContextModelId.BINPOSITION
        elif fun == 'TUbinsOrder1' or fun == 'EGkbinsOrder1':
            ctx_model_id = cabac.ContextModelId.BINSORDERN
        elif fun == 'TUsymbolOrder1' or fun == 'EGksymbolOrder1':
            ctx_model_id = cabac.ContextModelId.SYMBOLORDERN      


        # Encode
        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()
        t = self._tic()
        enc.encodeSymbols(symbols, bin_id, ctx_model_id, bin_params, ctx_params)
        t_enc3a = self._toc(t)

        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs3a = enc.getBitstream()

        # Decode
        dec = cabac.cabacSimpleSequenceDecoder(bs3a)
        dec.initCtx(num_ctxs, 0.5, 8)
        dec.start()

        t = self._tic()
        symbols_dec = dec.decodeSymbols(len(symbols), bin_id, ctx_model_id, bin_params, ctx_params)
        t_dec3a = self._toc(t)

        dec.decodeBinTrm()
        dec.finish()
        
        self.assertTrue((symbols_dec == symbols).all())
        self.assertTrue(bs1 == bs3a)


        bin_params = [num_max_val, k]
        ctx_params = [ctx_order, ctx_rest_pos, ctx_offset, ctx_symbol_max]

        if fun == 'TUBAC' or fun == 'EGkBAC':
            ctx_model_id = cabac.ContextModelId.BAC
        elif fun == 'TUBinPos' or fun == 'EGkBinPos':
            ctx_model_id = cabac.ContextModelId.BINPOSITION
        elif fun == 'TUbinsOrder1' or fun == 'EGkbinsOrder1':
            ctx_model_id = cabac.ContextModelId.BINSORDERN
        elif fun == 'TUsymbolOrder1' or fun == 'EGksymbolOrder1':
            ctx_model_id = cabac.ContextModelId.SYMBOLORDERN      


        ############################################################
        # 3.b) Encode sequence symbol-wise

        # Encode
        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()
        t = self._tic()
        symbol_prev = 0
        for i, symbol in enumerate(symbols):
            enc.encodeSymbol(symbol, [symbol_prev], bin_id, ctx_model_id, bin_params, ctx_params)
            symbol_prev = symbol
        t_enc3b = self._toc(t)

        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs3b = enc.getBitstream()

        # Decode
        dec = cabac.cabacSimpleSequenceDecoder(bs3b)
        dec.initCtx(num_ctxs, 0.5, 8)
        dec.start()

        t = self._tic()
        symbol_prev_dec = 0
        for i in range(0, num_symbols):            
            symbol_dec = dec.decodeSymbol([symbol_prev_dec], bin_id, ctx_model_id, bin_params, ctx_params)

            symbol_prev_dec = symbol_dec

            symbols_dec[i] = symbol_dec       
        
        t_dec3b = self._toc(t)

        dec.decodeBinTrm()
        dec.finish()
        
        self.assertTrue((symbols_dec == symbols).all())
        self.assertTrue(bs1 == bs3b)


        # Print overview
        print(
            f'Bitstream length: {len(bs1)}\n'
            f'Time overview:\n'
            f'Step 1  (bin)        : total time = {(t_enc1  + t_dec1 ):2.3f} s (enc: {t_enc1 :2.3f} s, dec: {t_dec1 :2.3f} s)\n'
            f'Step 2a (sym, ids)   : total time = {(t_enc2a + t_dec2a):2.3f} s (enc: {t_enc2a:2.3f} s, dec: {t_dec2a:2.3f} s)\n'
            f'Step 2b (sym, pyfun) : total time = {(t_enc2b + t_dec2b):2.3f} s (enc: {t_enc2b:2.3f} s, dec: {t_dec2b:2.3f} s)\n'
            f'Step 2c (sym, c-fun) : total time = {(t_enc2c + t_dec2c):2.3f} s (enc: {t_enc2c:2.3f} s, dec: {t_dec2c:2.3f} s)\n'
            f'Step 3a (sequence)   : total time = {(t_enc3a + t_dec3a):2.3f} s (enc: {t_enc3a:2.3f} s, dec: {t_dec3a:2.3f} s)\n'
            f'Step 3b (symbol-wise): total time = {(t_enc3b + t_dec3b):2.3f} s (enc: {t_enc3b:2.3f} s, dec: {t_dec3b:2.3f} s)\n'
        )

    def test_encode_symbols_timing(self):
        random.seed(0)
        print('test_encode_symbols_symbol_order1')
        funs = [
            'TUBAC', 'TUBinPos', 'TUbinsOrder1', 'TUsymbolOrder1',
            'EGkBAC', 'EGkBinPos', 'EGkbinsOrder1', 'EGksymbolOrder1'
        ]
        
        for fun in funs:
            print('Testing function: ' + fun)
            self._call_cabac_order1(fun)


if __name__ == "__main__":
    unittest.main()
