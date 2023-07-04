import unittest
import random
import cabac
import math

import tests.utils.symbolgenerator as symbolgenerator

class MainTest(unittest.TestCase):

    
    def _call_cabac_symbols_bac_binposition(self, fun='EGk'):
        import numpy as np
        ctx_order = 1  # not used here
        ctx_rest_pos = 24
        ctx_id_offset = 0
        symbol_max = 16
        num_max_val = 255
        num_max_prefix_val = int(math.floor(math.log2(num_max_val/(2**0) + 1)) + 1)
        num_bins = 8
        num_values = 10000
        num_bi_bins = 8
        
        k = 1
        symbols = symbolgenerator.create_random_symbols_uniform_distribution(num_values, num_max_val)
        symbols = symbolgenerator.create_random_symbols_geometric_distribution(num_values, 0.05)
        #symbols = list(range(0,8))

        num_values = len(symbols)
        symbols = np.array(symbols)
        
        bin_params = [num_max_val]
        ctx_params = [ctx_order, ctx_rest_pos, ctx_id_offset]
        
        if fun == 'BIBAC':
            bin_id = cabac.BinarizationId.BI
            ctx_model_id = cabac.ContextModelId.BAC

            bin_params = [num_bi_bins]

        elif fun == 'BIbinPosition':
            bin_id = cabac.BinarizationId.BI
            ctx_model_id = cabac.ContextModelId.BINPOSITION

            bin_params = [num_bi_bins]

        elif fun == 'TUBAC':
            bin_id = cabac.BinarizationId.TU
            ctx_model_id = cabac.ContextModelId.BAC
        
        elif fun == 'TUbinPosition':
            bin_id = cabac.BinarizationId.TU
            ctx_model_id = cabac.ContextModelId.BINPOSITION
                
        elif fun == 'EGkBAC':
            bin_id = cabac.BinarizationId.EGk
            ctx_model_id = cabac.ContextModelId.BAC

            bin_params = [num_max_val, k]

        elif fun == "EGkbinPosition":
            bin_id = cabac.BinarizationId.EGk
            ctx_model_id = cabac.ContextModelId.BINPOSITION

            bin_params = [num_max_val, k]

        num_ctxs = cabac.getNumContexts(bin_id, ctx_model_id, bin_params, ctx_params)

        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()

        enc.encodeSymbols(symbols, bin_id, ctx_model_id, bin_params, ctx_params)
                    
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        # Decode
        dec = cabac.cabacSimpleSequenceDecoder(bs)
        dec.initCtx(num_ctxs, 0.5, 8) 
        dec.start()

        decodedSymbols = dec.decodeSymbols(len(symbols), bin_id, ctx_model_id, bin_params, ctx_params)
        
        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs)))

        self.assertTrue((decodedSymbols == symbols).all())


    def test_encode_symbols_bac_binposition(self):
        random.seed(0)
        print('test_encode_symbols_bac_binposition')
        funs = [
            'BIBAC', 'BIbinPosition',
            'TUBAC', 'TUbinPosition',
            'EGkBAC', 'EGkbinPosition']

        for fun in funs:
            print('Testing function: ' + fun)
            self._call_cabac_symbols_bac_binposition(fun)


    def _call_cabac_symbols_order_n(self, fun='BIbinsOrderN', ctx_order=1):
        import numpy as np

        ctx_rest_pos = 24
        ctx_id_offset = 0
        symbol_max = 16
        num_max_val = 255
        num_max_prefix_val = int(math.floor(math.log2(num_max_val/(2**0) + 1)) + 1)
        num_bins = 8
        num_values = 10000
        num_bi_bins = 8
        
        k = 1
        
        symbols = symbolgenerator.create_random_symbols_geometric_distribution(num_values, 0.05)
        #symbols = list(range(0,8))

        num_values = len(symbols)
        symbols = np.array(symbols)
        
        bin_params = [num_max_val]
        ctx_params = [ctx_order, ctx_rest_pos, ctx_id_offset]
        if fun == 'BIbinsOrderN':
            bin_id = cabac.BinarizationId.BI
            ctx_model_id = cabac.ContextModelId.BINSORDERN

            bin_params = [num_bi_bins]

        elif fun == 'TUbinsOrderN':
            bin_id = cabac.BinarizationId.TU
            ctx_model_id = cabac.ContextModelId.BINSORDERN

        elif fun == 'TUsymbolOrderN':
            bin_id = cabac.BinarizationId.TU
            ctx_model_id = cabac.ContextModelId.SYMBOLORDERN

            ctx_params = [ctx_order, ctx_rest_pos, ctx_id_offset, symbol_max]

        elif fun == 'EGkbinsOrderN':
            bin_id = cabac.BinarizationId.EGk
            ctx_model_id = cabac.ContextModelId.BINSORDERN

            bin_params = [num_max_val, k]

        elif fun == 'EGksymbolOrderN':
            bin_id = cabac.BinarizationId.EGk
            ctx_model_id = cabac.ContextModelId.SYMBOLORDERN

            bin_params = [num_max_val, k]
            ctx_params = [ctx_order, ctx_rest_pos, 0, symbol_max]

        num_ctxs = cabac.getNumContexts(bin_id, ctx_model_id, bin_params, ctx_params)

        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()

        enc.encodeSymbols(symbols, bin_id, ctx_model_id, bin_params, ctx_params)
                    
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        # Decode
        dec = cabac.cabacSimpleSequenceDecoder(bs)
        dec.initCtx(num_ctxs, 0.5, 8) 
        dec.start()

        decodedSymbols = dec.decodeSymbols(len(symbols), bin_id, ctx_model_id, bin_params, ctx_params)
        
        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs)))

        self.assertTrue((decodedSymbols == symbols).all())


    def test_encode_symbols_order_n(self):
        random.seed(0)
        print('test_encode_symbols_order_n')
        funs = [
            'BIbinsOrderN',
            'TUbinsOrderN', 'TUsymbolOrderN',
            'EGkbinsOrderN', 'EGksymbolOrderN'
        ]
        #funs = ['EGkbinsOrderN', 'EGksymbolOrderN']

        for fun in funs:
            for order in [1, 2, 3]:
                print('Testing function: ' + fun + ' with order ' + str(order))
                self._call_cabac_symbols_order_n(fun, order)

    
    def _call_cabac_symbols_bypass(self, fun='EGk'):
        import numpy as np

        num_max_val = 255
        num_values = 10000
        num_bi_bins = 8
        
        k = 1
        
        symbols = symbolgenerator.create_random_symbols_uniform_distribution(num_values, num_max_val)
        symbols = symbolgenerator.create_random_symbols_geometric_distribution(num_values, 0.05)
        #symbols = list(range(0,8))

        num_values = len(symbols)
        symbols = np.array(symbols)
        
        bin_params = [num_max_val]
        if fun == 'BI':
            bin_id = cabac.BinarizationId.BI
            bin_params = [num_bi_bins]

        elif fun == 'TU':
            bin_id = cabac.BinarizationId.TU

        elif fun == 'EGk':
            bin_id = cabac.BinarizationId.EGk
            bin_params = [num_max_val, k]

        num_ctxs = 0

        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()

        enc.encodeSymbolsBypass(symbols, bin_id, bin_params)
                    
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        # Decode
        dec = cabac.cabacSimpleSequenceDecoder(bs)
        dec.initCtx(num_ctxs, 0.5, 8) 
        dec.start()

        decodedSymbols = dec.decodeSymbolsBypass(len(symbols), bin_id, bin_params)
        
        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs)))

        self.assertTrue((decodedSymbols == symbols).all())


    def test_encode_symbols_bypass_joint_fun(self):
        random.seed(0)
        print('test_encode_symbols_bypass_joint_fun')
        funs = ['BI', 'TU', 'EGk']
        
        for fun in funs:
            print('Testing function: ' + fun)
            self._call_cabac_symbols_bypass(fun)

if __name__ == "__main__":
    unittest.main()
